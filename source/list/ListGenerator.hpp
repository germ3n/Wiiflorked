/****************************************************************************
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef __LISTGENERATOR_HPP
#define __LISTGENERATOR_HPP

#include <string>
#include <vector>
#include <stdio.h>
#include <regex>

#include "defines.h"
#include "types.h"
#include "config/config.hpp"
#include "loader/wbfs.h"
#include "loader/disc.h"
#include "gui/GameTDB.hpp"
#include "plugin/plugin.hpp"

#define CONFIG_FILENAME_SKIP_DOMAIN "PLUGINS"
#define CONFIG_FILENAME_SKIP_KEY    "filename_skip_regex"
#define CONFIG_FILENAME_SKIP_DEFAULT    "((dis[ck]|tape|side|track)[ _-]([b-l][^a-z]|0*[2-9]|0*[1-9][0-9]))|(^disc2[.]iso$)|(^neogeo[.]zip$)|(^funboot[.]rom$)|(^(ecs|exec|grom)[.]bin$)"

class ListGenerator 
{
private:
    std::vector<dir_discHdr> m_buffer;

    std::string gameTDB_Path;
    std::string gameTDB_Language;
    std::string CustomTitlesPath;

public:
    u32 Magic;
    u32 Color;
    bool usePluginDBTitles; 

    ListGenerator();
    ~ListGenerator();

    void Init(const char *settingsDir, const char *Language, const char *plgnsDataDir, const std::string& fileNameSkipPattern);
    void Clear(void);
    void OpenConfigs();
    void CloseConfigs();
    
    // Core Loaders
    void FastLoadDB(const std::string& dbPath);
	void FastLoadMultiDB(const std::vector<std::string>& dbPaths, std::vector<dir_discHdr>& targetList);
    void CreateRomList(const char *platform, const string& romsDir, const std::vector<string>& FileTypes, const string& DBName, bool UpdateCache);
    void CreateList(u32 Flow, const string& Path, const std::vector<string>& FileTypes, const string& DBName, bool UpdateCache);
    void ParseScummvmINI(Config &ini, const char *Device, const char *datadir, const char *platform, const string& DBName, bool UpdateCache);
    void createSFList(u8 maxBtns, Config &m_sourceMenuCfg, const string& sourceDir);

    // Vector Overrides
    void clear() { m_buffer.clear(); }
    void reserve(u32 size) { m_buffer.reserve(size); }
    void push_back(const dir_discHdr& item) { m_buffer.push_back(item); }
    bool empty() const { return m_buffer.empty(); }
    size_t size() const { return m_buffer.size(); }

    dir_discHdr& operator[](u32 idx) { return m_buffer[idx]; }

    // Native Iterators for menu.cpp (Search and Sort)
    std::vector<dir_discHdr>::iterator begin() { return m_buffer.begin(); }
    std::vector<dir_discHdr>::iterator end() { return m_buffer.end(); }
};

typedef void (*FileAdder)(char *Path);
void GetFiles(const char *Path, const std::vector<string>& FileTypes, 
            FileAdder AddFile, bool CompareFolders, u32 max_depth = 2, u32 depth = 1);

extern ListGenerator m_cacheList;

#endif