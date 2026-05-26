#include "cache.hpp"

extern "C" void gprintf(const char *format, ...);

CCache::CCache(vector<dir_discHdr> &list, string path, CMode mode)
{
    filename = path;
    gprintf("CCache::CCache() - Opening %s in mode %s\n", filename.c_str(), mode == LOAD ? "LOAD" : "SAVE");

    cache = fopen(filename.c_str(), io[mode]);
    if(!cache) 
    {
        gprintf("CCache::CCache() - Failed to open %s\n", filename.c_str());
        return;
    }

    switch(mode)
    {
        case LOAD:
            LoadAll(list);
            break;
        case SAVE:
            SaveAll(list);
            break;
        default:
            return;
    }
}

CCache::~CCache()
{
    gprintf("CCache::~CCache() - Closing DB: %s\n", filename.c_str());
    if(cache) fclose(cache);
    cache = NULL;
}

void CCache::SaveAll(vector<dir_discHdr> list)
{
    gprintf("CCache::SaveAll() - Updating DB: %s (%u items)\n", filename.c_str(), list.size());
    if(!cache) 
    {
        gprintf("CCache::SaveAll() - DB handle is NULL, aborting save\n");
        return;
    }
    fwrite((void *)&list[0], 1, list.size() * sizeof(dir_discHdr), cache);
}

void CCache::LoadAll(vector<dir_discHdr> &list)
{
    if(!cache) return;

    gprintf("CCache::LoadAll() - Starting bulk load: %s\n", filename.c_str());

    // 1. Get file size efficiently
    fseek(cache, 0, SEEK_END);
    u64 fileSize = ftell(cache);
    rewind(cache); // Faster than fseek(cache, 0, SEEK_SET)

    u32 count = (u32)(fileSize / sizeof(dir_discHdr));
    if (count == 0) return;

    gprintf("CCache::LoadAll() - File size: %llu bytes. Games found: %u\n", fileSize, count);
    
    // 2. Pre-allocate the vector size so we don't copy memory while reading
    size_t oldSize = list.size();
    list.resize(oldSize + count);
    
    // 3. The Bulk Read: Stream the entire file into memory in one go
    // fread is now reading directly into the memory block of the vector
    size_t itemsRead = fread(&list[oldSize], sizeof(dir_discHdr), count, cache);
    
    // 4. Verification
    if(itemsRead != count) {
        gprintf("CCache::LoadAll() - ERROR: Expected %u, but read %u items!\n", count, itemsRead);
        list.resize(oldSize + itemsRead); // Shrink back to what we actually got
    }
    
    gprintf("CCache::LoadAll() - Successfully bulk-loaded %u items\n", itemsRead);
}