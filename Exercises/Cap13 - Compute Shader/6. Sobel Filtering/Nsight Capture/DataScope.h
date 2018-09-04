//--------------------------------------------------------------------------------------
// File: DataScope.h
//
// Distributed as part of NVIDIA Nsight serialization output.
//
// Copyright (c) NVIDIA Corporation.  All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include <vector>
#include <cassert>
#include <algorithm>
#include <memory>

#if defined(_MSC_VER) && _MSC_VER < 1900
    #define TLS_STORAGE __declspec(thread)
#elif defined(__GNUC__)
    #define TLS_STORAGE __thread
#else
    #define TLS_STORAGE thread_local
#endif

#define NV_DATABASE_WARN(CONDITION, MESSAGE) assert((CONDITION) && MESSAGE)

#define DATA_SCOPE_NAME_CONCAT_HELPER(X, Y) X##Y
#define DATA_SCOPE_NAME_CONCAT(X, Y) DATA_SCOPE_NAME_CONCAT_HELPER(X, Y)

#define BEGIN_DATA_SCOPE_FUNCTION_EX(DATABASE_CLASS) \
    auto& dataScopeTracker = Serialization::DataScopeTracker<DATABASE_CLASS>::Instance(); \
    BEGIN_DATA_SCOPE_EX(DATABASE_CLASS)

#define BEGIN_DATA_SCOPE_EX(DATABASE_CLASS) Serialization::DataScope<DATABASE_CLASS> DATA_SCOPE_NAME_CONCAT(scope_line_no_, __LINE__)(dataScopeTracker)

namespace Serialization
{

#if defined(__arm__)
struct BlobProxyBase
{
};
#endif // defined(__arm__)

template <typename T_Database>
class DataScopeTracker;

template <typename T_Database>
class DataScope
{
public:
    using PageId = uint64_t;

    DataScope(DataScopeTracker<T_Database>& tracker)
        : m_Tracker(tracker)
        , m_pParent()
        , m_staticStorage{}
        , m_dynamicStorage()
        , m_pUsedPages(m_staticStorage)
        , m_usedPageCount()
        , m_pDatabase()
    {
        m_pParent = m_Tracker.m_pCurrentDataScope;
        m_Tracker.m_pCurrentDataScope = this;
    }

    ~DataScope()
    {
        if (m_pDatabase) {
            for (int i = 0; i < m_usedPageCount; ++i) {
                m_pDatabase->LockOrUnlock(m_pUsedPages[i], false);
            }
        }

        m_Tracker.m_pCurrentDataScope = m_pParent;
    }

    void SetUsesPage(PageId pageOffset, T_Database& database)
    {
        if (LocalScopeUsesPage(pageOffset))
        {
            // This page is already tracked.
            return;
        }

        database.LockOrUnlock(pageOffset, true);

        if (m_usedPageCount < NUM_STATIC_IDS)
        {
            m_pUsedPages[m_usedPageCount] = pageOffset;
        }
        else
        {
            if (m_usedPageCount == NUM_STATIC_IDS)
            {
                m_dynamicStorage.assign(m_staticStorage, m_staticStorage + NUM_STATIC_IDS);
            }

            m_dynamicStorage.push_back(pageOffset);
            m_pUsedPages = m_dynamicStorage.data();
        }

        ++m_usedPageCount;
        std::sort(m_pUsedPages, m_pUsedPages + m_usedPageCount);

        NV_DATABASE_WARN(!m_pDatabase || m_pDatabase == &database, "There should be a single database.");
        m_pDatabase = &database;
    }

private:
    bool LocalScopeUsesPage(PageId pageOffset) const
    {
        if (0 == m_usedPageCount)
        {
            return false;
        }

        auto pEnd = m_pUsedPages + m_usedPageCount;
        auto pLoc = std::lower_bound(m_pUsedPages, pEnd, pageOffset);
        if (!pLoc || pLoc == pEnd)
        {
            return false;
        }

        return *pLoc == pageOffset;
    }

    DataScopeTracker<T_Database>& m_Tracker;
    DataScope<T_Database>* m_pParent;

    const static int NUM_STATIC_IDS = 2;
    PageId m_staticStorage[NUM_STATIC_IDS];
    std::vector<PageId> m_dynamicStorage;

    PageId* m_pUsedPages;
    int m_usedPageCount;
    T_Database* m_pDatabase;

#if defined(__arm__)
public:
    void AddBlobProxy(std::shared_ptr<BlobProxyBase>& pBlobProxy)
    {
        m_blobProxyVec.push_back(pBlobProxy);
    }
private:
    std::vector<std::shared_ptr<BlobProxyBase> > m_blobProxyVec;
#endif
};

// Singleton to track data usage scope.
template <typename T_Database>
class DataScopeTracker
{
    friend class DataScope<T_Database>;

public:
    static DataScopeTracker& Instance()
    {
        static TLS_STORAGE DataScopeTracker* pTracker = nullptr;
        DataScopeTracker* pLocal = pTracker;
        if (!pLocal) {
            pTracker = new DataScopeTracker();
            return *pTracker;
        }
        return *pLocal;
    }

    DataScopeTracker(const DataScopeTracker&) = delete;
    void operator=(const DataScopeTracker&) = delete;

    void SetUsesPage(uint64_t pageOffset, T_Database& database)
    {
        if (!m_pCurrentDataScope)
        {
            return;
        }
        m_pCurrentDataScope->SetUsesPage(pageOffset, database);
    }
#if defined(__arm__)
    void AddBlobProxyToCurrentDataScope(std::shared_ptr<BlobProxyBase>& pBlobProxy)
    {
        NV_DATABASE_WARN(m_pCurrentDataScope != nullptr, "Not find current data scope");
        if(m_pCurrentDataScope != nullptr)
        {
            m_pCurrentDataScope->AddBlobProxy(pBlobProxy);
        }
    }
#endif

private:
    DataScopeTracker()
        : m_pCurrentDataScope()
    {
    }

    DataScope<T_Database>* m_pCurrentDataScope;
};

} // namespace Serialization
