//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// 0.0.7
// Alexey Potehin <gnuplanet@gmail.com>, http://www.gnuplanet.ru/doc/cv
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <list>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "submodule/libcore.cpp/libcore.hpp"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#if (INTPTR_MAX != INT32_MAX) && (INTPTR_MAX != INT64_MAX)
#error "Environment not 32 or 64-bit."
#endif
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
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
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// split file to strings
int stage1(void *p_mmap, size_t size)
{
	char *p = (char *)p_mmap;

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
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// find patch tokens
int stage2(void *p_mmap, size_t size)
{
	char *p = (char *)p_mmap;

	std::list<global::item_t>::iterator iterator_prev  = global::item_list.end();

	std::list<global::item_t>::iterator iterator_minus = global::item_list.end();
	std::list<global::item_t>::iterator iterator_plus  = global::item_list.end();
	std::list<global::item_t>::iterator iterator_token = global::item_list.end();

	for (std::list<global::item_t>::iterator i=global::item_list.begin(), i_end = global::item_list.end(); i != i_end; ++i)
	{
#ifdef FLAG_DEBUG
#if (INTPTR_MAX == INT32_MAX)
		printf("%u\n", (*i).offset);
#endif
#if (INTPTR_MAX == INT64_MAX)
		printf("%lu\n", (*i).offset);
#endif
#endif

		size_t offset = (*i).offset;
		if ((offset + 3) >= size)
		{
			(*i).flag_valid = false;
			continue;
		}

		for (;;)
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
//					(*i).flag_valid=false;
					if ((iterator_plus == global::item_list.end()) || (iterator_token == global::item_list.end()))
					{
						(*iterator_minus).flag_valid = false;
					}
				}


				iterator_minus  = i;
				iterator_prev   = i;
				(*i).flag_minus = true;
#ifdef FLAG_DEBUG
#if (INTPTR_MAX == INT32_MAX)
				printf("detect minus %u\n", (*i).offset);
#endif
#if (INTPTR_MAX == INT64_MAX)
				printf("detect minus %lu\n", (*i).offset);
#endif
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
					(*i).flag_valid = false;
					break;
				}


				iterator_plus  = i;
//				iterator_prev  = i;
				(*i).flag_plus = true;
#ifdef FLAG_DEBUG
#if (INTPTR_MAX == INT32_MAX)
				printf("detect plus %u\n", (*i).offset);
#endif
#if (INTPTR_MAX == INT64_MAX)
				printf("detect plus %lu\n", (*i).offset);
#endif
#endif
				(*i).flag_valid = false; //!!!
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
					(*i).flag_valid = false;
					break;
				}


				iterator_token  = i;
				iterator_prev   = i;
				(*i).flag_token = true;
#ifdef FLAG_DEBUG
#if (INTPTR_MAX == INT32_MAX)
				printf("detect token %u\n", (*i).offset);
#endif
#if (INTPTR_MAX == INT64_MAX)
				printf("detect token %lu\n", (*i).offset);
#endif
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
#ifdef FLAG_DEBUG
#if (INTPTR_MAX == INT32_MAX)
					printf("diff token shirink, (*iterator_prev).offset=%u, (*i).offset=%u, (*iterator_prev).size=%u\n", (*iterator_prev).offset, (*i).offset, (*iterator_prev).size);
#endif
#if (INTPTR_MAX == INT64_MAX)
					printf("diff token shirink, (*iterator_prev).offset=%lu, (*i).offset=%lu, (*iterator_prev).size=%lu\n", (*iterator_prev).offset, (*i).offset, (*iterator_prev).size);
#endif
#endif
				}
				iterator_prev = i;

				(*i).flag_valid = false;
#ifdef FLAG_DEBUG
#if (INTPTR_MAX == INT32_MAX)
				printf("detect diff %u\n", (*i).offset);
#endif
#if (INTPTR_MAX == INT64_MAX)
				printf("detect diff %lu\n", (*i).offset);
#endif
#endif
				break;
			}

			(*i).flag_valid = false;
			break;
		}
	}

	if (iterator_prev != global::item_list.end())
	{
		(*iterator_prev).size = size - (*iterator_prev).offset;
	}

	if (iterator_minus != global::item_list.end())
	{
//		(*i).flag_valid = false;
		if ((iterator_plus == global::item_list.end()) || (iterator_token == global::item_list.end()))
		{
			(*iterator_minus).flag_valid = false;

			if (iterator_plus == global::item_list.end())
			{
#ifdef FLAG_DEBUG
				printf("iterator_plus == null\n");
#endif
			}
			if (iterator_token == global::item_list.end())
			{
#ifdef FLAG_DEBUG
				printf("iterator_token == null\n");
#endif
			}

		}
	}


	return 0;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// save tokens to files
int stage3(void *p_mmap, const std::string &file_name)
{
	int rc;
	char *p = (char *)p_mmap;

	size_t file_count  = -1;
	size_t token_count = -1;

	for (std::list<global::item_t>::iterator i=global::item_list.begin(), i_end = global::item_list.end(); i != i_end; ++i)
	{
		if ((*i).flag_valid == false) continue;
		if ((*i).flag_minus == false) continue;

#ifdef FLAG_DEBUG
#if (INTPTR_MAX == INT32_MAX)
		printf("minus, offset=%u, size=%u\n", (*i).offset, (*i).size);
#endif
#if (INTPTR_MAX == INT64_MAX)
		printf("minus, offset=%lu, size=%lu\n", (*i).offset, (*i).size);
#endif
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
				printf("plus, offset=%lu, size=%lu\n", (*i).offset, (*i).size);
			}
*/

			if ((*j).flag_minus == true)
			{
				break;
			}


			if ((*j).flag_token == true)
			{
#ifdef FLAG_DEBUG
#if (INTPTR_MAX == INT32_MAX)
				printf("token, offset=%u, size=%u\n", (*j).offset, (*j).size);
#endif
#if (INTPTR_MAX == INT64_MAX)
				printf("token, offset=%lu, size=%lu\n", (*j).offset, (*j).size);
#endif
				printf("{---------------------------------------------------------------------\n");
#endif

				if (global::flag_pedantic == true)
				{

					body = head + std::string(p + (*j).offset, (*j).size);
					token_count++;

#ifdef FLAG_DEBUG
					printf("%s", body.c_str());
#endif
					char buf[1024];
#if (INTPTR_MAX == INT32_MAX)
					sprintf(buf, "%s-%08u-%08u.patch", file_name.c_str(), file_count, token_count);
#endif
#if (INTPTR_MAX == INT64_MAX)
					sprintf(buf, "%s-%08lu-%08lu.patch", file_name.c_str(), file_count, token_count);
#endif
					rc = libcore::file_set(buf, 0, body.c_str(), body.size(), true, false);
					if (rc == -1)
					{
						printf("\nERROR[libcore::file_set()]: %s\n", strerror(errno));
						return -1;
					}
				}
				else
				{
					body += std::string(p + (*j).offset, (*j).size);
				}

#ifdef FLAG_DEBUG
				printf("---------------------------------------------------------------------}\n");
#endif
			}

		}

		if (global::flag_pedantic == false)
		{
			char buf[1024];
#if (INTPTR_MAX == INT32_MAX)
			sprintf(buf, "%s-%08u.patch", file_name.c_str(), file_count);
#endif
#if (INTPTR_MAX == INT64_MAX)
			sprintf(buf, "%s-%08lu.patch", file_name.c_str(), file_count);
#endif
			rc = libcore::file_set(buf, 0, body.c_str(), body.size(), true, false);
			if (rc == -1)
			{
				printf("\nERROR[libcore::file_set()]: %s\n", strerror(errno));
				return -1;
			}
		}
	}


	return 0;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
int split(const std::string &file_name, void *p, size_t size)
{
	if (stage1(p, size) != 0)
	{
		global::item_list.clear();
		return -1;
	}

	if (stage2(p, size) != 0)
	{
		global::item_list.clear();
		return -1;
	}

	if (stage3(p, file_name) != 0)
	{
		global::item_list.clear();
		return -1;
	}

	global::item_list.clear();
	return 0;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
int do_it(const std::string &file_name)
{
	int result;
	int rc;


// open file
	rc = open(file_name.c_str(), O_RDONLY);
	if (rc == -1)
	{
		printf("\nERROR[do_it::open]: %s\n", strerror(errno));
		return -1;
	}
	int file_handle = rc;


// get file size
	struct stat64 stat_buf;
	rc = fstat64(file_handle, &stat_buf);
	if (rc != 0)
	{
		printf("\nERROR[do_it::fstat]: %s\n", strerror(errno));
		return -1;
	}
	size_t size = (size_t)stat_buf.st_size;


// check file size
	if (size == 0)
	{
		printf("\nERROR[do_it]: file is empty\n");
		return -1;
	}


// map file to memory
	void *p_mmap = mmap(NULL, size, PROT_READ, MAP_PRIVATE, file_handle, 0);
	if (p_mmap == MAP_FAILED)
	{
		printf("\nERROR[do_it::mmap]: %s\n", strerror(errno));
		return -1;
	}


// do work
	result = split(file_name, p_mmap, size);


// close map file to memory
	rc = munmap(p_mmap, size);
	if (rc == -1)
	{
		printf("\nERROR[do_it::munmap]: %s\n", strerror(errno));
		return -1;
	}


// close file
	rc = close(file_handle);
	if (rc == -1)
	{
		printf("\nERROR[do_it::close]: %s\n", strerror(errno));
		return -1;
	}


	return result;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// view help
void help()
{
	printf ("%s    %s\n", PROG_FULL_NAME, PROG_URL);
	printf ("example: %s [-p, --flag_pedantic=true|false] file.patch\n", PROG_NAME);
	printf ("\n");
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// main function
int main(int argc, char *argv[])
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
			(strcmp(argv[1], "-h")     == 0) ||
			(strcmp(argv[1], "-help")  == 0) ||
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
				libcore::str2bool(global::flag_pedantic, false, value);
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
		printf("Ok.\n");
	}


	return rc;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
