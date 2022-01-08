#pragma once

#ifndef HAVE_TIMEGM

#include <ctime>

time_t timegm(struct tm * tm);

#endif
