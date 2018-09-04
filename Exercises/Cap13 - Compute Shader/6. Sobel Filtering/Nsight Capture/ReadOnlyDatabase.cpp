//--------------------------------------------------------------------------------------
// File: ReadOnlyDatabase.cpp
//
// Distributed as part of NVIDIA Nsight serialization output.
//
// Copyright (c) NVIDIA Corporation.  All rights reserved.
//--------------------------------------------------------------------------------------

#include "ReadOnlyDatabase.h"
#include <algorithm>
#include <cassert>
#include <string>

#if NV_ANDROID_EXTERNAL()
#include "Helpers.h"
#include <android/asset_manager.h>
#endif

namespace Serialization {
//------------------------------------------------------------------------------
// Definition for DATABASE_HANDLE_INVALID
//------------------------------------------------------------------------------
const DATABASE_HANDLE DATABASE_HANDLE_INVALID;

//------------------------------------------------------------------------------
// CloseFile
//------------------------------------------------------------------------------
static void CloseFile(NVFILE* pFile)
{
#if NV_ANDROID_EXTERNAL()
    AAsset_close(pFile);
#else
    fclose(pFile);
#endif
}

//------------------------------------------------------------------------------
// OpenFile
//------------------------------------------------------------------------------
static NVFILE* OpenFile(const char* pFileName)
{
#if NV_ANDROID_EXTERNAL()
    return AAssetManager_open(pAndroidApp->activity->assetManager, pFileName, AASSET_MODE_STREAMING);
#else
    return fopen(pFileName, "rb");
#endif
}

//------------------------------------------------------------------------------
// SeekInFile
//------------------------------------------------------------------------------
static bool SeekInFile(NVFILE* pFile, uint64_t offset, int whence)
{
#if NV_ANDROID_EXTERNAL()
    return AAsset_seek(pFile, offset, whence) != -1;
#elif defined(__ANDROID__) || defined(__QNX__) || defined(__INTEGRITY)
    // Android/QNX does not have LFS
    return !fseek(pFile, offset, whence);
#elif defined(_WIN32)
    return !_fseeki64(pFile, offset, whence);
#else
    return !fseeko64(pFile, offset, whence);
#endif
}

//------------------------------------------------------------------------------
// GetFileLength
//------------------------------------------------------------------------------
static size_t GetFileLength(NVFILE* pFile)
{
    size_t totalSize = 0;
    NV_DATABASE_WARN(pFile, "Invalid file");

#if NV_ANDROID_EXTERNAL()
    totalSize = (size_t)AAsset_getLength(pFile);
#elif defined(__ANDROID__) || defined(__QNX__) || defined(__INTEGRITY)
    // Android/QNX does not have LFS
    fseek(pFile, 0, SEEK_END);
    totalSize = (size_t)ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
#elif defined(_WIN32)
    _fseeki64(pFile, 0, SEEK_END);
    totalSize = (size_t)_ftelli64(pFile);
    _fseeki64(pFile, 0, SEEK_SET);
#else
    fseeko64(pFile, 0, SEEK_END);
    totalSize = (size_t)ftello64(pFile);
    fseeko64(pFile, 0, SEEK_SET);
#endif
    return totalSize;
}

//------------------------------------------------------------------------------
// ReadFromFile
//------------------------------------------------------------------------------
static size_t ReadFromFile(NVFILE* pFile, size_t elemSize, size_t numElements, void* buf)
{
    NV_DATABASE_WARN(pFile, "Invalid file");

#if NV_ANDROID_EXTERNAL()
    const int bytesRead = AAsset_read(pFile, buf, elemSize * numElements);
    return (std::max)(size_t(bytesRead), size_t(0));
#else
    const size_t elemsRead = fread(buf, elemSize, numElements, pFile);
    return elemSize * elemsRead;
#endif
}

//------------------------------------------------------------------------------
// Constructors.
//------------------------------------------------------------------------------
ReadOnlyDatabase::ReadOnlyDatabase()
    : m_pFile(NULL)
    , m_PageAccessCounter(0)
    , m_PageSizeThreshold(1 << 20) // Default: 1 MB per page
    , m_MaxResidentPages(256) // Default: 256 pages
    , m_ForceEvict(false)
{
}

ReadOnlyDatabase::ReadOnlyDatabase(const char* pFileName, uint64_t PageSizeThreshold, size_t MaxResidentPages)
    : m_pFile(NULL)
    , m_PageAccessCounter(0)
    , m_PageSizeThreshold(PageSizeThreshold)
    , m_MaxResidentPages(MaxResidentPages)
    , m_ForceEvict(false)
{
    InitReadOnly(pFileName);
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
ReadOnlyDatabase::~ReadOnlyDatabase()
{
    CleanupPageRecords();
    if (m_pFile)
    {
        CloseFile(m_pFile);
    }
}

//------------------------------------------------------------------------------
// Init- assign a filename.
//------------------------------------------------------------------------------
bool ReadOnlyDatabase::Init(const char* pFileName)
{
    return InitReadOnly(pFileName);
}

void ReadOnlyDatabase::FreeCachedMemory()
{
    // Quick return if we have nothing resident
    if (m_ResidentBlobPages.empty())
    {
        return;
    }

    // Clear resident pages and dealloc memory if needed.
    std::vector<BlobPage*>::iterator iter = m_ResidentBlobPages.begin();
    std::vector<BlobPage*>::iterator end = m_ResidentBlobPages.end();
    for (; iter != end; ++iter)
    {
        BlobPage* pPage = *iter;
        if (pPage->pMemory)
        {
            delete[] pPage->pMemory;
            pPage->pMemory = NULL;
        }
    }

    m_ResidentBlobPages.clear();
}

//------------------------------------------------------------------------------
// Build up our internally stored pages.
//------------------------------------------------------------------------------
void ReadOnlyDatabase::BuildPageRecords()
{
    // Make sure we are clean to begin with.
    CleanupPageRecords();

    // Our raw blob records should already be initialized.  Loop through and construct
    // pages.  We want the pages to be less than or equal to m_PageSizeThreshold.
    // If a single blob exceeds m_PageSizeThreshold, then it gets its own page.

    uint64_t currentPageOffset = 0;
    uint64_t currentPageSize = 0;
    std::vector<BlobRecord>::iterator iter = m_BlobRecords.begin();
    std::vector<BlobRecord>::iterator end = m_BlobRecords.end();
    for (; iter != end; ++iter)
    {
        const BlobRecord& record = *iter;

        // Determine if there are gaps in the generated resource file records
        // and adjust the current page size to account for them
        uint64_t recordGap = record.Offset - (currentPageOffset + currentPageSize);
        if (recordGap)
        {
            currentPageSize += recordGap;
        }

        if (currentPageSize && currentPageSize + record.Size > m_PageSizeThreshold)
        {
            // This page is big enough already.
            m_BlobPages.push_back(BlobPage(currentPageOffset, currentPageSize));

            currentPageOffset = record.Offset;
            currentPageSize = 0;
        }

        currentPageSize += record.Size;
    }

    // Final element.
    if (currentPageSize)
    {
        m_BlobPages.push_back(BlobPage(currentPageOffset, currentPageSize));
    }
}

//------------------------------------------------------------------------------
// Clean up our internally stored pages.
//------------------------------------------------------------------------------
void ReadOnlyDatabase::CleanupPageRecords()
{
    // Clear resident pages
    FreeCachedMemory();

    // Clear non-resident page info
    m_BlobPages.clear();
}

//------------------------------------------------------------------------------
// InitReadOnly- assign a filename (readonly specific).
//------------------------------------------------------------------------------
bool ReadOnlyDatabase::InitReadOnly(const char* pFileName)
{
    // Assume success
    bool retval = true;
    NVFILE* pRecordsFile = NULL;

    CleanupPageRecords();
    m_BlobRecords.clear();
    m_BlobPages.clear();

    if (m_pFile)
    {
        CloseFile(m_pFile);
        m_pFile = NULL;
    }

    if (!pFileName)
    {
        NV_DATABASE_WARN(false, "pFileName must be non-null");
        return false;
    }

        // Handle both when the database is in the current directory as well as
        // the parent folder (or parent's parent folder). Also ensure the first
        // search path is empty to handle the possibility that the path is
        // absolute. nullptr must be the last entry.
#if defined(_WIN32)
    const char* directoriesToTry[] = { "", ".\\", "..\\", "..\\..\\", nullptr };
#else
    const char* directoriesToTry[] = { "", "../", "../../", nullptr };
#endif
    std::string pathToFile;
    for (auto directoryIndex = 0; directoriesToTry[directoryIndex]; ++directoryIndex)
    {
        pathToFile = std::string(directoriesToTry[directoryIndex]) + std::string(pFileName);
        m_pFile = OpenFile(pathToFile.c_str());
        if (m_pFile)
        {
            // Update pFileName given the path
            pFileName = pathToFile.c_str();
            break;
        }
    }

    if (!m_pFile)
    {
        auto fileOpenWarning = "Could not open file" + pathToFile;
        NV_DATABASE_WARN(false, fileOpenWarning.c_str());
        return false;
    }

    std::string RecordsFileName = std::string(pFileName) + ".rec";

    try
    {
        // Now read the table that will tell us how the blobs are layed out
        pRecordsFile = OpenFile(RecordsFileName.c_str());
        if (!pRecordsFile)
        {
            auto recordOpenWarning = "Can't open records" + RecordsFileName;
            NV_DATABASE_WARN(false, recordOpenWarning.c_str());
            throw 0;
        }

        BuildBlobRecords(pRecordsFile);

        BuildPageRecords();
    }
    catch (std::bad_alloc&)
    {
        retval = false;
    }
    catch (...)
    {
        retval = false;
    }

    // Cleanup.
    if (pRecordsFile)
    {
        CloseFile(pRecordsFile);
    }

    return retval;
}

//------------------------------------------------------------------------------
// BuildBlobRecords - Build up the blob records from an open file
//------------------------------------------------------------------------------
void ReadOnlyDatabase::BuildBlobRecords(NVFILE* pFile)
{
    // Read the count, by getting the total file size
    const size_t totalSize = GetFileLength(pFile);

    size_t recordsCount = totalSize / sizeof(BlobRecord);
    m_BlobRecords.resize(recordsCount);

    // Read the records
    if ((recordsCount > 0) && (0 == ReadFromFile(pFile, sizeof(m_BlobRecords[0]), recordsCount, &m_BlobRecords[0])))
    {
        NV_DATABASE_WARN(false, "Can't read records");
        throw 0;
    }
}

//------------------------------------------------------------------------------
// Clean up our internally stored blob table
//------------------------------------------------------------------------------
void ReadOnlyDatabase::CleanupBlobRecords()
{
    // Clear blob records.
    m_BlobRecords.clear();
}

//------------------------------------------------------------------------------
// GetSize - Get the size of a blob if it exists, or zero
//------------------------------------------------------------------------------
uint64_t ReadOnlyDatabase::GetSize(const DATABASE_HANDLE& Handle)
{
    if (Handle.value < 0 || Handle.value >= (int)m_BlobRecords.size())
    {
        return 0;
    }

    return m_BlobRecords[Handle.value].Size;
}

//------------------------------------------------------------------------------
// DoRead - Read paged blob from the database.  Returns NULL upon error.
//------------------------------------------------------------------------------
void* ReadOnlyDatabase::DoRead(const DATABASE_HANDLE& handle)
{
    if (DATABASE_HANDLE_INVALID == handle)
    {
        return NULL;
    }

    else if (handle.value < 0 || handle.value >= (int)m_BlobRecords.size())
    {
        NV_DATABASE_WARN(false, "Invalid handle");
        return NULL;
    }

    const BlobRecord& neededBlob = m_BlobRecords[handle.value];
    BlobPage* pPage = NULL;

    try
    {
        // First to see if the needed page is resident.
        BlobPage pageKey(neededBlob.Offset, neededBlob.Size);
        std::vector<BlobPage>::iterator foundIter;
        foundIter = std::lower_bound(m_BlobPages.begin(), m_BlobPages.end(), pageKey, PageOffsetComparator());

        if (m_BlobPages.end() == foundIter || neededBlob.Offset < foundIter->PageOffset || neededBlob.Offset >= foundIter->PageOffset + foundIter->PageSize)
        {
            NV_DATABASE_WARN(false, "Could not find page matching requested blob");
            return NULL;
        }

        pPage = &(*foundIter);

        // Track the usage of this page in this scope
        auto& scopeTracker = DataScopeTracker<ReadOnlyDatabase>::Instance();
        scopeTracker.SetUsesPage(pPage->PageOffset, *this);

        // Cache miss.
        if (!pPage->pMemory)
        {
            // We should only need to lock as many pages as the number of blob params consumed by a single function call.
            // Ten should be more than conservative.
            const static size_t MAX_EXPECTED_LOCKED_PAGES = 10;

            uint8_t* pRecycledMemory = NULL;
            uint64_t RecycledMemorySize = 0;

            auto PagesNeedEvicting = [this]() -> bool {
                // If we are in a "Force Evict" situation, we want to clear out all of the resident pages
                return m_ResidentBlobPages.size() && (m_ForceEvict || m_ResidentBlobPages.size() >= m_MaxResidentPages);
            };

            // If necessary, search for unlocked pages to evict.  The oldest pages should be at the beginning of the vector
            if (PagesNeedEvicting())
            {
                // Sort resident pages by last access
                std::sort(m_ResidentBlobPages.begin(), m_ResidentBlobPages.end(), PageLastAccessComparator());

                std::vector<BlobPage*>::iterator pageIter = m_ResidentBlobPages.begin();
                while (PagesNeedEvicting() && pageIter != m_ResidentBlobPages.end())
                {
                    BlobPage* pEvictPage = *pageIter;
                    if (pEvictPage->LockCount <= 0)
                    {
                        NV_DATABASE_WARN(pEvictPage->LockCount == 0, "Attempting to evict locked page");

                        // If we are not in the "m_ForceEvict" path, recycle the allocation if possible
                        if (!m_ForceEvict && NULL == pRecycledMemory && pEvictPage->MemoryCapacity <= m_PageSizeThreshold && pEvictPage->MemoryCapacity >= pPage->PageSize)
                        {
                            pRecycledMemory = pEvictPage->pMemory;
                            RecycledMemorySize = pEvictPage->MemoryCapacity;
                        }
                        else
                        {
                            delete[] pEvictPage->pMemory;
                        }

                        pEvictPage->pMemory = NULL;
                        pageIter = m_ResidentBlobPages.erase(pageIter);
                    }
                    else
                    {
                        ++pageIter;
                    }
                }
            }

            NV_DATABASE_WARN(m_ResidentBlobPages.size() < (size_t)std::max(m_MaxResidentPages, MAX_EXPECTED_LOCKED_PAGES), "Too many resident pages.");

            if (pRecycledMemory)
            {
                pPage->pMemory = pRecycledMemory;
                pPage->MemoryCapacity = RecycledMemorySize;
            }
            else
            {
                pPage->pMemory = new uint8_t[(size_t)pPage->PageSize];
                pPage->MemoryCapacity = pPage->PageSize;
            }

            // Insert in resident list
            m_ResidentBlobPages.insert(m_ResidentBlobPages.end(), pPage);

            // Read the page data from disk.
            SeekInFile(m_pFile, pPage->PageOffset, SEEK_SET);
            const uint64_t bytesRead = ReadFromFile(m_pFile, size_t(pPage->PageSize), 1, pPage->pMemory);

            if (pPage->PageSize != bytesRead)
            {
                NV_DATABASE_WARN(false, "Could not read page from file");
                return NULL;
            }
        }
    }
    catch (std::bad_alloc&)
    {
        NV_DATABASE_WARN(false, "Out of memory!");
        return NULL;
    }
    catch (...)
    {
        NV_DATABASE_WARN(false, "Unknown error");
        return NULL;
    }

    if (!pPage || !pPage->pMemory)
    {
        NV_DATABASE_WARN(false, "Missing data");
        return NULL;
    }

    uint64_t localOffset = neededBlob.Offset - pPage->PageOffset;
    pPage->LastAccessCounter = m_PageAccessCounter++;

    // Return an offset into the paged data.
    return pPage->pMemory + localOffset;
}

//------------------------------------------------------------------------------
// LockOrUnlock - increment or decrement the refcount on this blob (and the page that contains it)
//------------------------------------------------------------------------------
void ReadOnlyDatabase::LockOrUnlock(uint64_t pageOffset, bool lock)
{
    BlobPage pageKey(pageOffset, 0);
    std::vector<BlobPage>::iterator foundIter;
    foundIter = std::lower_bound(m_BlobPages.begin(), m_BlobPages.end(), pageKey, PageOffsetComparator());

    if (m_BlobPages.end() == foundIter || pageOffset < foundIter->PageOffset)
    {
        NV_DATABASE_WARN(false, "Could not find page matching requested blob");
        return;
    }

    // Found it!
    if (lock)
    {
        NV_DATABASE_WARN(foundIter->LockCount >= 0, "Lock count should not be less than zero");
        foundIter->LockCount++;
    }
    else
    {
        NV_DATABASE_WARN(foundIter->LockCount > 0, "Lock count should be greater than zero before we decrement it");
        foundIter->LockCount--;
    }
}

//------------------------------------------------------------------------------
// Less-than operator for BlobPage.  If this returns false for both (a,b) and
// (b,a) this means that one memory span contains the other.
//------------------------------------------------------------------------------
int ReadOnlyDatabase::PageOffsetComparator::operator()(const BlobPage& item1, const BlobPage& item2)
{
    return (item1.PageOffset < item2.PageOffset) && (item1.PageOffset + item1.PageSize < item2.PageOffset + item2.PageSize);
}

//------------------------------------------------------------------------------
// Less-than operator for BlobPage.  If this returns false for both (a,b) and
// (b,a) this means that one memory span contains the other.
//------------------------------------------------------------------------------
int ReadOnlyDatabase::PageLastAccessComparator::operator()(const BlobPage* item1, const BlobPage* item2)
{
    return (item1->LastAccessCounter < item2->LastAccessCounter);
}

} // namespace Serialization
