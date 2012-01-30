//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// 0.0.6
// Alexey Potehin, http://www.gnuplanet.ru/doc/cv, <gnuplanet@gmail.com>
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#include <stdio.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <list>
#include <unistd.h>
#include <algorithm>

//#define DEBUG
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// global variables
namespace global
{
    struct item_t
    {
	size_t offset;
	size_t size;
	bool flag_minus;
	bool flag_plus;
	bool flag_token;
	bool flag_valid;

	item_t(size_t offset)
	{
	    this->offset     = offset;
	    this->size       = 0;
	    this->flag_minus = false;
	    this->flag_plus  = false;
	    this->flag_token = false;
	    this->flag_valid = true;
	}
    };

    std::list<global::item_t> item_list;

    bool flag_pedantic = false;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// convert string to boolean
bool str2bool(const std::string& str)
{
    std::string tmp = str;

    std::transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);

    if (tmp == "true")
    {
	return true;
    }

    if (tmp == "1")
    {
	return true;
    }

    return false;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// save file
int save_file(const std::string& file_name, const unsigned char* const p, size_t size)
{
    int rc;


// open new file
    umask(0);
    rc = ::open(file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (rc == -1)
    {
	printf ("\nERROR[save_file::open]: %s\n", strerror(errno));
	return -1;
    }
    int file_handle = rc;


// write file
    size_t offset = 0;
    for (;;)
    {
	if (offset == size) break;
	rc = ::write(file_handle, p + offset, size - offset);
	if (rc == -1)
	{
	    printf ("\nERROR[save_file::write]: %s\n", strerror(errno));
	    return -1;
	}
	offset += rc;
    }


// flush file
    rc = ::fsync(file_handle);
    if (rc != 0)
    {
	printf ("\nERROR[save_file::fsync]: (%d)%s\n", errno, strerror(errno));
	return -1;
    }


// close file
    rc = ::close(file_handle);
    if (rc != 0)
    {
	printf ("\nERROR[save_file::close]: (%d)%s\n", errno, strerror(errno));
	return -1;
    }


    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// split file to strings
int stage1(void* p_mmap, size_t size)
{
    char* p = (char*)p_mmap;

    global::item_list.push_back(0);
    for (size_t i=0; i < size; i++)
    {
	if (*p++ == '\n')
	{
	    global::item_list.push_back(global::item_t(i + 1));
	}
    }

    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// find patch tokens
int stage2(void* p_mmap, size_t size)
{
    char* p = (char*)p_mmap;

    std::list<global::item_t>::iterator iterator_prev  = global::item_list.end();

    std::list<global::item_t>::iterator iterator_minus = global::item_list.end();
    std::list<global::item_t>::iterator iterator_plus  = global::item_list.end();
    std::list<global::item_t>::iterator iterator_token = global::item_list.end();

    for (std::list<global::item_t>::iterator i=global::item_list.begin(), i_end = global::item_list.end(); i != i_end; ++i)
    {
#ifdef DEBUG
	printf ("%lu\n", (*i).offset);
#endif

	size_t offset = (*i).offset;
	if ((offset + 3) >= size)
	{
	    (*i).flag_valid=false;
	    continue;
	}

	for(;;)
	{
	    if
	    (
		(p[offset + 0] == '-') &&
		(p[offset + 1] == '-') &&
		(p[offset + 2] == '-') &&
		(p[offset + 3] == ' ')
	    )
	    {
		if (iterator_prev != global::item_list.end())
		{
		    (*iterator_prev).size = (*i).offset - (*iterator_prev).offset;
		}

		if (iterator_minus != global::item_list.end())
		{
//		    (*i).flag_valid=false;
		    if ((iterator_plus == global::item_list.end()) || (iterator_token == global::item_list.end()))
		    {
			(*iterator_minus).flag_valid = false;
		    }
		}


		iterator_minus = i;
		iterator_prev = i;
		(*i).flag_minus=true;
#ifdef DEBUG
		printf ("detect minus %lu\n", (*i).offset);
#endif
		break;
	    }

	    if
	    (
		(p[offset + 0] == '+') &&
		(p[offset + 1] == '+') &&
		(p[offset + 2] == '+') &&
		(p[offset + 3] == ' ')
	    )
	    {
/*
		if (iterator_prev != global::item_list.end())
		{
		    (*iterator_prev).size = (*i).offset - (*iterator_prev).offset;
		}
*/
		if (iterator_minus == global::item_list.end())
		{
		    (*i).flag_valid=false;
		    break;
		}


		iterator_plus = i;
//		iterator_prev = i;
		(*i).flag_plus=true;
#ifdef DEBUG
		printf ("detect plus %lu\n", (*i).offset);
#endif
		(*i).flag_valid=false; //!!!
		break;
	    }

	    if
	    (
		(p[offset + 0] == '@') &&
		(p[offset + 1] == '@') &&
		(p[offset + 2] == ' ')
	    )
	    {
		if (iterator_prev != global::item_list.end())
		{
		    (*iterator_prev).size = (*i).offset - (*iterator_prev).offset;
		}

		if (iterator_minus == global::item_list.end())
		{
		    (*i).flag_valid=false;
		    break;
		}


		iterator_token = i;
		iterator_prev = i;
		(*i).flag_token=true;
#ifdef DEBUG
		printf ("detect token %lu\n", (*i).offset);
#endif
		break;
	    }

	    if // git patch
	    (
		(p[offset + 0] == 'd') &&
		(p[offset + 1] == 'i') &&
		(p[offset + 2] == 'f')
	    )
	    {
		if (iterator_prev != global::item_list.end())
		{
		    (*iterator_prev).size = (*i).offset - (*iterator_prev).offset;
#ifdef DEBUG
		    printf ("diff token shirink, (*iterator_prev).offset=%lu, (*i).offset=%lu, (*iterator_prev).size=%lu\n", (*iterator_prev).offset, (*i).offset, (*iterator_prev).size);
#endif
		}
		iterator_prev = i;

		(*i).flag_valid=false;
#ifdef DEBUG
		printf ("detect diff %lu\n", (*i).offset);
#endif
		break;
	    }

	    (*i).flag_valid=false;
	    break;
	}
    }

    if (iterator_prev != global::item_list.end())
    {
	(*iterator_prev).size = size - (*iterator_prev).offset;
    }

    if (iterator_minus != global::item_list.end())
    {
//	(*i).flag_valid=false;
	if ((iterator_plus == global::item_list.end()) || (iterator_token == global::item_list.end()))
	{
	    (*iterator_minus).flag_valid = false;

	    if (iterator_plus == global::item_list.end())
	    {
#ifdef DEBUG
		printf("iterator_plus == null\n");
#endif
	    }
	    if (iterator_token == global::item_list.end())
	    {
#ifdef DEBUG
		printf("iterator_token == null\n");
#endif
	    }

	}
    }



    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// save tokens to files
int stage3(void* p_mmap, const std::string& file_name)
{
    int rc;
    char* p = (char*)p_mmap;

    size_t file_count = -1;
    size_t token_count = -1;

    for (std::list<global::item_t>::iterator i=global::item_list.begin(), i_end = global::item_list.end(); i != i_end; ++i)
    {
	if ((*i).flag_valid == false) continue;
	if ((*i).flag_minus == false) continue;

#ifdef DEBUG
	printf ("minus, offset=%lu, size=%lu\n", (*i).offset, (*i).size);
#endif
	std::string body = "";
	std::string head(p + (*i).offset, (*i).size);

	if (global::flag_pedantic == false)
	{
	    body = head;
	}

	file_count++;

	for (std::list<global::item_t>::iterator j=++i; j != i_end; ++j)
	{
/*
	    if ((*i).flag_plus == true)
	    {
		printf ("plus, offset=%lu, size=%lu\n", (*i).offset, (*i).size);
	    }
*/

	    if ((*j).flag_minus == true)
	    {
		break;
	    }


	    if ((*j).flag_token == true)
	    {
#ifdef DEBUG
		printf ("token, offset=%lu, size=%lu\n", (*j).offset, (*j).size);
		printf ("{---------------------------------------------------------------------\n");
#endif

		if (global::flag_pedantic == true)
		{

		    body = head + std::string(p + (*j).offset, (*j).size);
		    token_count++;

#ifdef DEBUG
		    printf ("%s", body.c_str());
#endif
		    char buf[1024];
		    sprintf(buf, "%s-%08lu-%08lu.patch", file_name.c_str(), file_count, token_count);
		    rc = save_file(std::string(buf), (const unsigned char*)body.c_str(), body.size());
		    if (rc == -1) return -1;
		}
		else
		{
		    body += std::string(p + (*j).offset, (*j).size);
		}

#ifdef DEBUG
		printf ("---------------------------------------------------------------------}\n");
#endif
	    }

	}

	if (global::flag_pedantic == false)
	{
	    char buf[1024];
	    sprintf(buf, "%s-%08lu.patch", file_name.c_str(), file_count);
	    rc = save_file(std::string(buf), (const unsigned char*)body.c_str(), body.size());
	    if (rc == -1) return -1;
	}
    }

    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
int do_it(const std::string& file_name)
{
    int rc;


// open file
    rc = open(file_name.c_str(), O_RDONLY);
    if (rc == -1)
    {
	printf ("\nERROR[do_it::open]: %s\n", strerror(errno));
	return -1;
    }
    int file_handle = rc;


// get file size
    struct stat64 stat_buf;
    rc = fstat64(file_handle, &stat_buf);
    if (rc != 0)
    {
	printf ("\nERROR[do_it::fstat]: %s\n", strerror(errno));
	return -1;
    }
    size_t size = (size_t)stat_buf.st_size;


// check file size
    if (size == 0)
    {
	printf ("\nERROR[do_it]: file is empty\n");
	return -1;
    }


// map file to memory
    void *p_mmap = mmap(NULL, size, PROT_READ, MAP_PRIVATE, file_handle, 0);
    if (p_mmap == MAP_FAILED)
    {
	printf ("\nERROR[do_it::mmap]: %s\n", strerror(errno));
	return -1;
    }


// do work
    stage1(p_mmap, size);
    stage2(p_mmap, size);
    stage3(p_mmap, file_name);


// close map file to memory
    rc = munmap(p_mmap, size);
    if (rc == -1)
    {
	printf ("\nERROR[do_it::munmap]: %s\n", strerror(errno));
	return -1;
    }


// close file
    rc = close(file_handle);
    if (rc == -1)
    {
	printf ("\nERROR[do_it::close]: %s\n", strerror(errno));
	return -1;
    }


    return 0;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void help()
{
    printf ("patch_spliter    version %s-%s\n", ARCH, VERSION);
    printf ("example: patch_spliter [-p, --flag_pedantic=true|false] file.patch\n");
    printf ("\n");
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// main function
int main(int argc, char* argv[])
{
    int rc;


    if ((argc == 1) || (argc > 3))
    {
	help();
	return 1;
    }


    if (argc == 2)
    {
	if
	(
	    (strcmp(argv[1], "-h") == 0) ||
	    (strcmp(argv[1], "-help") == 0) ||
	    (strcmp(argv[1], "--help") == 0)
	)
	{
	    help();
	    return 1;
	}
	rc = do_it(argv[1]);
    }
    else
    {
	std::string key = argv[1];

	{
	    std::string tmpl = "--flag_pedantic=";
	    if ((key.size() >= tmpl.size()) && (key.substr(0, tmpl.size()) == tmpl))
	    {
		std::string value = key.substr(tmpl.size(), key.size() - 1);
		global::flag_pedantic = str2bool(value);
	    }
	}

	{
	    std::string tmpl = "-p";
	    if (key == tmpl)
	    {
		global::flag_pedantic = true;
	    }
	}

	rc = do_it(argv[2]);
    }


    if (rc == 0)
    {
	printf ("Ok.\n");
    }


    return rc;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
