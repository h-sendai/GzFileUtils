// -*- C++ -*-
/*!
 * @file GzFileUtils.h
 * @brief File operation utilities for Logger Comp using gz compression
 * @date
 * @author Kazuo Nakayoshi <kazuo.nakayoshi@kek.jp>
 * @author Hiroshi Sendai  <hiroshi.sendai@kek.jp>
 *
 * Copyright (C) 2011, 2016
 *     Kazuo Nakayoshi, Hiroshi Sendai
 *     Electronics System Group,
 *     KEK, Japan.
 *     All rights reserved.
 *
 */

#ifndef GZFILEUTILS_H
#define GZFILEUTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <exception>
#include <cerrno>
#include <unistd.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <zlib.h>

struct GzFileInfo {
    gzFile file;
    // std::ofstream* file;
    std::string name_main;
    std::string file_path;
    unsigned long long size;
    int branch_no;
    unsigned int run_no;
};

class GzFileUtils
{
public:
    GzFileUtils();
    GzFileUtils(std::string ext_name);
    virtual ~GzFileUtils();

    bool check_dir(std::string dir_name);
    void set_extension(std::string ext_name);
    int  set_max_size_in_megaBytes(unsigned int size);
    int  set_run_no(unsigned int run_no);
    int  write_data(char* data, unsigned long size);
    int  open_file(std::string dir_name);
    int  open_file(std::string dir_name, char* stream_buf,
                   unsigned int buf_size);
    int  close_file();

private:
    void set_max_size(unsigned long long size);
    int  open_file_incr_branch(std::string dir_name);
    int  open_file_incr_branch(std::string dir_name, char* stream_buf,
                               unsigned int buf_size);
    void incr_branch_no();
    void reset_branch_no();
    void reset_file_size();
    std::string get_date_time();
    std::string gen_file_name(bool incr_branch = false);

    static const unsigned int MAX_RUN_NO = 999999;
    unsigned long long m_max_size;
    GzFileInfo m_file_info;
    std::string m_ext_name;
    std::string m_dir_name;
    bool m_auto_fname;
    bool m_debug;
};

#endif
