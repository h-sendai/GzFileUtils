// -*- C++ -*-
/*!
 * @file GzFileUtils.cpp
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

#include "GzFileUtils.h"

/*
 * @class GzFileUtils
 * @brief File operation utilites for data logging using gz compression
 *
 * Function: Genarate files and names using run no, date and time(ISO string).
 *           File splitting by specified size, with branch no.
 *           External buffer can be used for iostream as default one.
 *
 * Data file name:
 *  case: run no <= 0
 *   YYYYMMDD T HHMMSS _ 000000 _ BBB . (file extension)
 *                             e.g. 20110202T143748_000000_000.dat
 *
 *  case: run no > 0 and run no < 999999
 *   YYYYMMDD T HHMMSS _ NNNNNN _ BBB . (file extension)
 *                             e.g. 20110202T143748_000100_000.dat
 *
 *   YYYY: year, MM: moth, DD: day, HH: hour, MM: minute, SS: second
 *   NNNNNN: run no, BBB: file branch no
 *
 * This class provides public interface as follows.
 *  - check_dir():     Check on whether specified directory exists.
 *  - set_extension(): Set file extension.
 *  - set_max_size_in_megaBytes(): Set max file size for file splitting
 *  - set_run_no(): Set run no for file name.
 *  - open_file(dir_name): Open file with specified directory
 *  - open_file(dir_name, stream_buf, buf_size): Open file with specified
 *    directory and specified external buffer for file stream.
 *  - close(): Close file
 */

GzFileUtils::GzFileUtils()
    : m_max_size(0), m_ext_name("dat"), m_dir_name(""),
      m_auto_fname(false), m_debug(false)
{
    if (m_debug) {
        std::cerr << "GzFileUtils create\n";
    }
    m_file_info.file = 0;
    m_file_info.name_main = "";
    m_file_info.size = 0;
    m_file_info.branch_no = 0;
    m_file_info.run_no = 0;
}

GzFileUtils::GzFileUtils(const std::string ext_name)
    : m_max_size(0), m_ext_name(ext_name), m_dir_name(""),
      m_auto_fname(false), m_debug(false)
{
    if (m_debug) {
        std::cerr << "GzFileUtils create\n";
    }
    m_file_info.file = 0;
    m_file_info.name_main = "";
    m_file_info.size = 0;
    m_file_info.branch_no = 0;
    m_file_info.run_no = 0;
}

GzFileUtils::~GzFileUtils()
{
    if (m_debug) {
        std::cerr << "FileUtils deleted\n";
    }
}

bool GzFileUtils::check_dir(std::string dir_name)
{
    boost::filesystem::path mydir(dir_name);
    if (boost::filesystem::exists(mydir)) {
        if (boost::filesystem::is_directory(mydir)) {
            if (access(dir_name.c_str(), W_OK) == 0) {
                return true;
            }
            else {
                std::cerr << "### ERROR: directory is not writable"
                          << std::endl;
            }
        }
    }
    return false;
}

void GzFileUtils::set_extension(std::string ext_name)
{
    m_ext_name = ext_name;
}

void GzFileUtils::set_max_size(unsigned long long size)
{
    m_max_size = size;
}

int GzFileUtils::set_max_size_in_megaBytes(unsigned int size)
{
    if (size < 0 || size > 1024) {
        return -1;
    }
    unsigned long long msize = size * 1024 * 1024;
    set_max_size(msize);
    std::cerr << "set max size in bytes:" << m_max_size << std::endl;
    return 0;
}

int GzFileUtils::set_run_no(unsigned int run_no)
{
    if (run_no > MAX_RUN_NO) {
        return -1;
    }
    m_file_info.run_no = run_no;
    if (run_no <= 0) {
        m_auto_fname = true;
    }
    else {
        m_auto_fname = false;
    }
    return 0;
}

int GzFileUtils::write_data(char* data, unsigned long size)
{

    //if (!m_file_info.file->write(data, size)) {
    int ret = gzwrite(m_file_info.file, data, size);
    if (ret < 0) {
    // if (!m_file_info.file->write(data, size)) {
        std::cerr << "### ERROR:" << errno << std::endl;
        perror("write_data");
        close_file();
        return -1;
    }
    // XXX: Raw data size (not compressed size)
    //      If we zlib >= 1.2.3.5, we can use gzoffset(gzFile)
    m_file_info.size += size;
#if ZLIB_VERNUM >= 0x1235
    m_file_info.compresed_size = gzoffset(m_file_info.file);
#endif
//  int current_file_size = boost::filesystem::file_size(m_file_info.file_path);
//  if ((m_max_size > 0) && (m_max_size <= current_file_size)) {
//      close_file();
//      open_file_incr_branch(m_dir_name);
//  }

    if ((m_max_size > 0) && (m_max_size <= m_file_info.size)) {
        close_file();
        open_file_incr_branch(m_dir_name);
    }
    return 0;
}

std::string GzFileUtils::get_date_time()
{
    boost::posix_time::ptime now =
        boost::posix_time::second_clock::local_time();
    return boost::posix_time::to_iso_string(now);
}

void GzFileUtils::incr_branch_no()
{
    m_file_info.branch_no += 1;
}

void GzFileUtils::reset_branch_no()
{
    m_file_info.branch_no = 0;
}

void GzFileUtils::reset_file_size()
{
    m_file_info.size = 0;
}

int GzFileUtils::open_file(std::string dir_name)
{
    if (check_dir(dir_name) == false) {
        return -1;
    }

    m_dir_name = dir_name;

    reset_branch_no();
    reset_file_size();
    std::string fileName = gen_file_name();
    //m_file_info.name = dir_name + "/" + fileName;
    m_file_info.file_path = dir_name + "/" + fileName + ".gz";

    //std::ofstream* outFile = new std::ofstream();
    //// outFile->open(m_file_info.name.c_str());
    //outFile->open(m_file_info.file_path.c_str());
    //m_file_info.file = outFile;
    
    gzFile outFile;
    outFile = gzopen(m_file_info.file_path.c_str(), "w");
    m_file_info.file = outFile;

    if (!m_file_info.file) {
        std::cerr << "### ERROR: open file: error occured\n";
        return -1;
    }
    return 0;
}

int GzFileUtils::open_file(std::string dir_name, char* stream_buf,
                         unsigned int buf_size)
{
    if (check_dir(dir_name) == false) {
        return -1;
    }

    m_dir_name = dir_name;

    reset_branch_no();
    reset_file_size();
    std::string fileName = gen_file_name();
    m_file_info.file_path = dir_name + "/" + fileName + ".gz";

    //std::ofstream* outFile = new std::ofstream();
    //outFile->rdbuf()->pubsetbuf(stream_buf, buf_size);
    //outFile->open(m_file_info.file_path.c_str());
    //m_file_info.file = outFile;

    gzFile outFile;
    outFile = gzopen(m_file_info.file_path.c_str(), "w");
    m_file_info.file = outFile;

    if (!m_file_info.file) {
        std::cerr << "### ERROR: open file: error occured\n";
        return -1;
    }

#if ZLIB_VERNUM >= 0x1235
    if (gzbuffer(outFile, buf_size) < 0) {
        std::cerr << "### ERROR: gzbuffer fail" << std::endl;
        return -1;
    }
#endif

    return 0;
}

int GzFileUtils::close_file()
{
    int rv = gzclose(m_file_info.file);
    if (rv == 0) {
        return 0;
    }
    else {
        perror("gzfile close error");
        return -1;
    }
        
    //m_file_info.file->close();
    //if (m_file_info.file) {
    //    return 0;
    //}
    //else {
    //    std::cerr << "### ERROR: close file: error occured\n";
    //    return -1;
    //}
}

int GzFileUtils::open_file_incr_branch(std::string dir_name)
{
    m_dir_name = dir_name;
    reset_file_size();

    std::string fileName = gen_file_name(true);
    m_file_info.file_path = dir_name + "/" + fileName + ".gz";

    // std::ofstream* outFile = new std::ofstream();
    // outFile->open(m_file_info.file_path.c_str());
    // m_file_info.file = outFile;
    
    gzFile outFile = gzopen(m_file_info.file_path.c_str(), "w");
    m_file_info.file = outFile;

    if (!m_file_info.file) {
        std::cerr << "### ERROR: open file: error occured\n";
        return -1;
    }
    return 0;
}

int GzFileUtils::open_file_incr_branch(std::string dir_name, char* stream_buf,
                                     unsigned int buf_size)
{
    m_dir_name = dir_name;
    reset_file_size();

    std::string fileName = gen_file_name(true);
    m_file_info.file_path = dir_name + "/" + fileName + ".gz";

    // std::ofstream* outFile = new std::ofstream();
    // outFile->rdbuf()->pubsetbuf(stream_buf, buf_size);
    // outFile->open(m_file_info.file_path.c_str());

    gzFile outFile = gzopen(m_file_info.file_path.c_str(), "w");
    m_file_info.file = outFile;

    if (!m_file_info.file) {
        std::cerr << "### ERROR: open file: error occured\n";
        return -1;
    }

#if ZLIB_VERNUM >= 0x1235
    if (gzbuffer(m_file_info.file, buf_size) < 0) {
        std::cerr << "### ERROR: gzbuffer() error occured\n";
        return -1;
    }
#endif
        
    return 0;
}

std::string GzFileUtils::gen_file_name(bool incr_branch)
{
    std::stringstream run_no;
    std::stringstream file_br_no;
    std::string date_time = "";
    std::string myconnector = "_";
    std::string mydot = ".";

    std::string fileName = "";

    if (incr_branch) { //Open a file for same run, increment branch no.
        incr_branch_no();
    }
    else {             //Open a file for new run, no increment branch no.
        date_time = get_date_time();
        m_file_info.name_main = date_time;
    }

    file_br_no << std::setw(3)
               << std::setfill('0')
               << m_file_info.branch_no;

    if (!m_auto_fname) {
        run_no << std::setw(6)
               << std::setfill('0')
               << m_file_info.run_no;

        fileName = m_file_info.name_main
                   + myconnector
                   + run_no.str()
                   + myconnector
                   + file_br_no.str()
                   + mydot
                   + m_ext_name;
    }
    else {
        fileName = m_file_info.name_main
                   + myconnector
                   + file_br_no.str()
                   + mydot
                   + m_ext_name;
    }

    if (m_debug) {
        std::cerr << "m_file_info.branch_no:"
                  << m_file_info.branch_no << std::endl;
        std::cerr << "file_no.str():" << file_br_no.str() << std::endl;
        std::cerr << "m_run_no:" << m_file_info.run_no << std::endl;
        std::cerr << "file name:" << fileName << std::endl;
    }
    return fileName;
}

