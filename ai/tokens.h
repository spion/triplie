/*
 *      tokens.h
 *
 *      Copyright 2008 Gorgi Kosev <spion@spion.ws>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */
#pragma once
        /*  A bit unclean. Besides being a token-functions file this also
                contains some standard math functions that are in use,
                and some types that hold our data.
         */
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <map>
#include <list>
#include <math.h>
#include <ctime>
using namespace std;

struct CAdminList {
    vector<string> userhosts;
};

double uniform_deviate(int seed);

void tokenize(const string& str,
        vector<string>& tokens,
        const string& delimiters = " ");

void tokenizelossles(const string& str,
        vector<string>& tokens,
        const string& delimiters = " ");

string gettok(const std::string& str, long int n,
        const std::string& delimiters = " ");

long int numtok(const std::string& str,
        const std::string& delimiters = " ");

long int findtok(const std::string& str, const std::string& token,
        const std::string& delimiters = " ");

string subtokstring(const vector<string>& tokens, unsigned int n1,
        unsigned int n2, const string& c);

void lowercase(string& s);
bool find_in_words(string& s, string t);