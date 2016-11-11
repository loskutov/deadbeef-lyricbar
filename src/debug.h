#pragma once
#ifndef LYRICBAR_DEBUG_H
#define LYRICBAR_DEBUG_H

#include <iostream>

#ifdef _DEBUG
#define debug_out std::cerr
#else
#define debug_out \
    if (true) {} \
    else std::cerr
#endif

#endif // LYRICBAR_DEBUG_H
