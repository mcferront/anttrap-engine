

template <typename t_key, typename t_data>
bool Enumerator<t_key, t_data>::EnumNext( void )
{
    return pHash->EnumNext( this );
}

template <typename t_key, typename t_data>
HashTable<t_key, t_data>::HashTable( void )
{
    m_pHashFunction = NULL;
}


template <typename t_key, typename t_data>
HashTable<t_key, t_data>::~HashTable( void )
{
    Debug::Assert( Condition( NULL == m_pHashFunction ), "HashTable was not properly destroyed" );
}

template <typename t_key, typename t_data>
void HashTable<t_key, t_data>::Create(
    size_t startItems, //= 16
    size_t growBy, //= 16
    HashFunction pHash, //= StringRefHash
    CompareFunction pCompare //= StringRefCompare
    )
{
    Debug::Assert( Condition( NULL == m_pHashFunction ), "HashTable was already created" );

    m_GrowBy = growBy;
    m_NumItems = startItems;

    m_ItemPool.Create( startItems );

    m_pHashFunction = pHash;
    m_pCompareFunction = pCompare;

    memset( m_Buckets, 0, sizeof( m_Buckets ) );

    m_WriteLock = 0;
}

template <typename t_key, typename t_data>
void HashTable<t_key, t_data>::Destroy( void )
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );
    Debug::Assert( Condition( 0 == m_WriteLock ), "Hashtable has been write locked, it cannot be modified" );

    m_ItemPool.Destroy( );

    m_pHashFunction = NULL;
}

template <typename t_key, typename t_data>
void HashTable<t_key, t_data>::Add(
    const t_key &key,
    const t_data &data
    )
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );
    Debug::Assert( Condition( 0 == m_WriteLock ), "Hashtable has been write locked, it cannot be modified" );

    uint32 hashKey, hash;

    hash = m_pHashFunction( key );
    hashKey = hash % BucketCount;

    Item *pPrevItem = NULL;
    Item *pItem = m_Buckets[ hashKey ].pItems;

    //go through all the bucket items
    //and see if we have one which is already using our key
    while ( pItem )
    {
        if ( m_pCompareFunction( pItem->key, key ) )
        {
            Debug::Print( Debug::TypeWarning, "Key Value Pair already added to hashtable\n" );
            break;
        }

        //save previous item so we can link
        //if we need to create a new item
        pPrevItem = pItem;
        pItem = pItem->pNextInBucket;
    }

    //no matching keys, create a new item
    //and link with the previous
    if ( NULL == pItem )
    {
        pItem = GetFreeItem( );
        pItem->pNextInBucket = NULL;

        //first in the bucket
        if ( NULL == pPrevItem )
        {
            m_Buckets[ hashKey ].pItems = pItem;
        }
        else
        {
            //link with the old in the bucket
            pPrevItem->pNextInBucket = pItem;
        }
    }

    //whether a new item was created
    //or we are using one w/ the matching key
    //copy our new data over
    memcpy( &pItem->data, &data, sizeof( t_data ) );
    memcpy( &pItem->key, &key, sizeof( t_key ) );
}

template <typename t_key, typename t_data>
void HashTable<t_key, t_data>::Remove(
    const t_key &key
    )
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );
    Debug::Assert( Condition( 0 == m_WriteLock ), "Hashtable has been write locked, it cannot be modified" );

    uint32 hash, hashKey;

    hash = m_pHashFunction( key );
    hashKey = hash % BucketCount;

    Item *pPrevBucketItem = NULL;
    Item *pBucketItem = m_Buckets[ hashKey ].pItems;

    while ( pBucketItem )
    {
        if ( m_pCompareFunction( pBucketItem->key, key ) )
        {
            //restore link with previous bucket item
            if ( pPrevBucketItem )
            {
                pPrevBucketItem->pNextInBucket = pBucketItem->pNextInBucket;
            }
            else
            {
                //if there wasn't a previous bucket item
                //then restore the forward link from the head
                m_Buckets[ hashKey ].pItems = pBucketItem->pNextInBucket;
            }

            FreeItem( pBucketItem );
            return;
        }

        pPrevBucketItem = pBucketItem;
        pBucketItem = pBucketItem->pNextInBucket;
    }
}

template <typename t_key, typename t_data>
bool HashTable<t_key, t_data>::Remove(
    const t_key &key,
    t_key *pKey,
    t_data *pData
    )
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );
    Debug::Assert( Condition( 0 == m_WriteLock ), "Hashtable has been write locked, it cannot be modified" );

    t_key *pPointerKey;
    t_data *pPointerData;

    if ( GetPointer( key, &pPointerKey, &pPointerData ) )
    {
        if ( NULL != pKey )
            memcpy( pKey, pPointerKey, sizeof( t_key ) );

        if ( NULL != pData )
            memcpy( pData, pPointerData, sizeof( t_data ) );

        Remove( key );
        return true;
    }

    return false;
}

template <typename t_key, typename t_data>
void HashTable<t_key, t_data>::Copy(
    const HashTable<t_key, t_data> &copyFrom
    )
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );
    Debug::Assert( Condition( 0 == m_WriteLock ), "Hashtable has been write locked, it cannot be modified" );

    Clear( );

    Enumerator<t_key, t_data> e = copyFrom.GetEnumerator( );
    while ( e.EnumNext( ) )
    {
        Add( e.Key( ), e.Data( ) );
    }
}

template <typename t_key, typename t_data>
bool HashTable<t_key, t_data>::Get(
    const t_key &key,
    t_data *pData
    ) const
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );

    t_key *pPointerKey;
    t_data *pPointerData;

    if ( GetPointer( key, &pPointerKey, &pPointerData ) )
    {
        memcpy( pData, pPointerData, sizeof( t_data ) );
        return true;
    }

    return false;
}

template <typename t_key, typename t_data>
bool HashTable<t_key, t_data>::Contains(
    const t_key &key
    ) const
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );

    uint32 hash, hashKey;

    hash = m_pHashFunction( key );
    hashKey = hash % BucketCount;

    Item *pBucketItem = m_Buckets[ hashKey ].pItems;

    while ( pBucketItem )
    {
        if ( m_pCompareFunction( pBucketItem->key, key ) )
        {
            return true;
        }

        pBucketItem = pBucketItem->pNextInBucket;
    }

    return false;
}

template <typename t_key, typename t_data>
void HashTable<t_key, t_data>::Clear( void )
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );
    Debug::Assert( Condition( 0 == m_WriteLock ), "Hashtable has been write locked, it cannot be modified" );

    m_ItemPool.Reset( );

    memset( m_Buckets, 0, sizeof( m_Buckets ) );
}

template <typename t_key, typename t_data>
bool HashTable<t_key, t_data>::GetPointer(
    const t_key &key,
    t_key **pKey,
    t_data **pData
    ) const
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );

    uint32 hash, hashKey;

    hash = m_pHashFunction( key );
    hashKey = hash % BucketCount;

    Item *pBucketItem = m_Buckets[ hashKey ].pItems;

    while ( pBucketItem )
    {
        if ( m_pCompareFunction( pBucketItem->key, key ) )
        {
            ( *pData ) = &pBucketItem->data;
            ( *pKey ) = &pBucketItem->key;
            return true;
        }

        pBucketItem = pBucketItem->pNextInBucket;
    }

    return false;
}

template <typename t_key, typename t_data>
bool HashTable<t_key, t_data>::EnumNext(
    Enumerator<t_key, t_data> *pEnumerator
    ) const
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );

    if ( NULL == pEnumerator->pItem )
    {
        uint32 i;

        for ( i = pEnumerator->bucket + 1; i < BucketCount; i++ )
        {
            if ( m_Buckets[ i ].pItems )
            {
                pEnumerator->pItem = m_Buckets[ i ].pItems;
                pEnumerator->bucket = i;
                break;
            }
        }

        if ( i == BucketCount ) return false;
    }
    else
    {
        pEnumerator->pItem = ( (Item *) pEnumerator->pItem )->pNextInBucket;

        if ( NULL == pEnumerator->pItem )
        {
            return EnumNext( pEnumerator );
        }
    }

    pEnumerator->pKey = &( (Item *) pEnumerator->pItem )->key;
    pEnumerator->pData = &( (Item *) pEnumerator->pItem )->data;

    return true;
}

template <typename t_key, typename t_data>
typename HashTable<t_key, t_data>::Item *HashTable<t_key, t_data>::GetFreeItem( void )
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );
    return m_ItemPool.Alloc( );
}

template <typename t_key, typename t_data>
void HashTable<t_key, t_data>::FreeItem(
    Item *pItem
    )
{
    Debug::Assert( Condition( NULL != m_pHashFunction ), "HashTable has not been created" );
    m_ItemPool.Free( pItem );
}
