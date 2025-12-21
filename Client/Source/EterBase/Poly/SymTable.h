#pragma once

#include <string>

class CSymTable
{
    public:
        CSymTable(int aTok, std::string aStr);
        virtual ~CSymTable();

        double		dVal;
        int		token;
        std::string	strlex;
};
