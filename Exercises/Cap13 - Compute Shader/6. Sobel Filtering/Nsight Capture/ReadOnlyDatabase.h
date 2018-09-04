//--------------------------------------------------------------------------------------
// File: ReadOnlyDatabase.h
//
// Distributed as part of NVIDIA Nsight serialization output.
//
// Copyright (c) NVIDIA Corporation.  All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DataScope.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <atomic>

#define BEGIN_DATA_SCOPE_FUNCTION() BEGIN_DATA_SCOPE_FUNCTION_EX(Serialization::ReadOnlyDatabase)
#define BEGIN_DATA_SCOPE() BEGIN_DATA_SCOPE_EX(Serialization::ReadOnlyDatabase)

#define NV_ANDROID_EXTERNAL() !defined(GTI_PROJECT) && defined(__ANDROID__) && !defined(__MINKE__)

#if NV_ANDROID_EXTERNAL()
struct AAsset;
typedef AAsset NVFILE;
#else
#include <cstdio>
typedef FILE NVFILE;
#endif

namespace Serialization {
// Forward declarations
class ReadOnlyDatabase;

// Declare handle type
struct DATABASE_HANDLE
{
    DATABASE_HANDLE()
    {
        value = ~0;
    }
    DATABASE_HANDLE(int32_t payload)
    {
        value = payload;
    }
    DATABASE_HANDLE& operator=(const DATABASE_HANDLE& other)
    {
        this->value = other.value;
        return *this;
    }
    bool operator==(const DATABASE_HANDLE& other) const
    {
        return this->value == other.value;
    }
    bool operator!=(const DATABASE_HANDLE& other) const
    {
        return this->value != other.value;
    }
    int32_t value;
};
const extern DATABASE_HANDLE DATABASE_HANDLE_INVALID;

// Declare BlobProxy: struct to auto-manage the lifetime of database blobs, passed directly as parameters to function calls.
template <typename T>
#if defined(__arm__)
struct BlobProxy : public BlobProxyBase
#else
struct BlobProxy
#endif// defined(__arm__)
{
    BlobProxy()
        : m_pDatabase(NULL)
        , m_pData(NULL)
    {
    }

    BlobProxy(ReadOnlyDatabase* pDatabase, void* pData, const DATABASE_HANDLE& hBlob)
        : m_pDatabase(pDatabase)
        , m_pData(pData)
        , m_hBlob(hBlob)
    {
#if defined(__arm__)
        AlignData();
#endif // defined(__arm__)
    }

    BlobProxy(const BlobProxy& rh)
    {
        *this = rh;
    }

    BlobProxy& operator=(const BlobProxy& rh)
    {
        m_pDatabase = rh.m_pDatabase;
        m_pData = rh.m_pData;
        m_hBlob = rh.m_hBlob;

        return *this;
    }

    ~BlobProxy()
    {
    }

    T Get()
    {
#if defined(__arm__)
        if (m_pAlignedData)
        {
            return reinterpret_cast<T>(m_pAlignedData);
        }
#endif // defined(__arm__)
        return reinterpret_cast<T>(m_pData);
    }

    T operator->()
    {
        return Get();
    }

    bool operator!()
    {
        return !Get();
    }

private:
    ReadOnlyDatabase* m_pDatabase;
    void* m_pData;
    DATABASE_HANDLE m_hBlob;

#if defined(__arm__)
    void AlignData();

    double* m_pAlignedData = nullptr;
    std::vector<double> alignedData;
#endif // defined(__arm__)
};

//----------------------------------------------------------------------------------
// ReadOnlyDatabase
//
// This class provides read access to a named blob database
//----------------------------------------------------------------------------------
class ReadOnlyDatabase
{
public:
    //------------------------------------------------------------------------------
    // Constructors
    //------------------------------------------------------------------------------
    ReadOnlyDatabase();
    ReadOnlyDatabase(const char* pFileName, uint64_t PageSizeThreshold, size_t MaxResidentPages);

    //------------------------------------------------------------------------------
    // Destructor- this will clear any results which have not been finalized.
    //------------------------------------------------------------------------------
    virtual ~ReadOnlyDatabase();

    //------------------------------------------------------------------------------
    // SetForceEvict- if true, this will force all pages to be evicted on the next
    // allocation
    //------------------------------------------------------------------------------
    void SetForceEvict(bool forceEvict)
    {
        m_ForceEvict = forceEvict;
    }

    //------------------------------------------------------------------------------
    // SetFileName- this indicates where we want to store results, if any.  If any previous
    // results are not finalized, they will be lost.  Passing NULL will result in a temporary
    // file, that will be deleted along with this object.
    //------------------------------------------------------------------------------
    virtual bool Init(const char* pFileName);

    //------------------------------------------------------------------------------
    // GetSize - Get the size of a blob if it exists, or zero
    //------------------------------------------------------------------------------
    uint64_t GetSize(const DATABASE_HANDLE& handle);

    //------------------------------------------------------------------------------
    // Read - Read paged blob from the database.  Returns a proxy which will either
    // return the memory, or NULL upon error.
    //------------------------------------------------------------------------------
    template <typename T>
    BlobProxy<T> Read(const DATABASE_HANDLE& handle)
    {
        void* pData = DoRead(handle);
        return BlobProxy<T>(this, pData, handle);
    }

    //------------------------------------------------------------------------------
    // ReadShared - Read paged blob from the database.  Returns a proxy shared_ptr
    // which will either return the memory, or NULL upon error.
    // Since arm device may need aligned data as member variable of BlobProxy, so use
    // this function to make sure aligned data is persist even out ReadShared function.
    //------------------------------------------------------------------------------
#if defined(__arm__)
    template <typename T>
    std::shared_ptr<BlobProxy<T> > ReadShared(const DATABASE_HANDLE& handle)
    {
        void* pData = DoRead(handle);
        return std::make_shared<BlobProxy<T> >(this, pData, handle);
    }
#endif

    // Helpers for Read
    void LockOrUnlock(uint64_t pageOffset, bool lock);

    //------------------------------------------------------------------------------
    // FreeCachedMemory - Clear the cache
    //------------------------------------------------------------------------------
    void FreeCachedMemory();

protected:
    // This class is non-copyable, no default constructor
    ReadOnlyDatabase(const ReadOnlyDatabase&);
    ReadOnlyDatabase& operator=(const ReadOnlyDatabase&);

    // Helper function for readonly-specific init
    bool InitReadOnly(const char* pFileNameReadOnly);

    // Build up the blob records from an open file
    void BuildBlobRecords(NVFILE* pFile);

    // Clean up our internally stored blob table
    void CleanupBlobRecords();

    // Build up our internally stored pages
    void BuildPageRecords();

    // Clean up our internally stored pages
    void CleanupPageRecords();

    // Helpers for Read
    void* DoRead(const DATABASE_HANDLE& handle);

    // The file.
    NVFILE* m_pFile;

    // Structure for tracking blobs in the database
    struct BlobRecord
    {
        uint64_t Size;
        uint64_t Offset;
    };

    // Table of blob records
    std::vector<BlobRecord> m_BlobRecords;

private:
    ///////////////////////////////////////////////////////////
    // Read mode memory tracking ("paging")
    ///////////////////////////////////////////////////////////
    struct BlobPage
    {
        BlobPage(uint64_t offset, uint64_t size)
            : PageOffset(offset)
            , PageSize(size)
            , LastAccessCounter()
            , pMemory()
            , MemoryCapacity()
            , LockCount()
        {
        }

        // Implement move constructor- this will only be called at startup when no pages are locked
        BlobPage(BlobPage&& rh)
            : PageOffset(rh.PageOffset)
            , PageSize(rh.PageSize)
            , LastAccessCounter(rh.LastAccessCounter)
            , pMemory(rh.pMemory)
            , MemoryCapacity(rh.MemoryCapacity)
            , LockCount(rh.LockCount.load())
        {
        }

        uint64_t PageOffset;
        uint64_t PageSize;
        uint64_t LastAccessCounter;
        uint8_t* pMemory;
        uint64_t MemoryCapacity;
        std::atomic<int32_t> LockCount;
    };

    //------------------------------------------------------------------------------
    // Helper class to binary search a BlobPage
    // This will indicate equality if one memory span contains the other.
    //------------------------------------------------------------------------------
    struct PageOffsetComparator
    {
        int operator()(const BlobPage& item1, const BlobPage& item2);
    };

    struct PageLastAccessComparator
    {
        int operator()(const BlobPage* item1, const BlobPage* item2);
    };

    std::vector<BlobPage> m_BlobPages;
    std::vector<BlobPage*> m_ResidentBlobPages; // Resident in memory

    uint64_t m_PageAccessCounter;

    // To conserve memory the database is read in "pages", although these are not
    // 4K pages in the virtual memory sense.  A page contains either a single binary
    // blob which is larger than m_PageSizeThreshold, or multiple blobs that total less than
    // m_PageSizeThreshold bytes.
    uint64_t m_PageSizeThreshold;

    // We'll set a limit on the number of pages that can be resident simultaneously.
    size_t m_MaxResidentPages;

    // This flag will force all pages to be evicted on the next allocation
    bool m_ForceEvict;
};

#if defined(__arm__)
template <typename T>
void BlobProxy<T>::AlignData()
{
    // Make sure reads from the proxy are appropriate for reading doubles
    const auto alignment = sizeof(double);
    if ((reinterpret_cast<uintptr_t>(m_pData) & (alignment - 1)) != 0)
    {
        // The data is not properly aligned.  Make a copy of it that is properly aligned
        const auto size = m_pDatabase->GetSize(m_hBlob);
        const auto alignedSize = (size + alignment - 1) / sizeof(double);
        alignedData.resize(alignedSize);
        m_pAlignedData = alignedData.data();
        memcpy(m_pAlignedData, m_pData, size);
    }
}
#endif // defined(__arm__)
} // namespace Serialization
