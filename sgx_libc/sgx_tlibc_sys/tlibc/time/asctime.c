/*	$OpenBSD: asctime.c,v 1.16 2010/08/23 22:35:34 millert Exp $ */
/*
** This file is in the public domain, so clarified as of
** 1996-06-05 by Arthur David Olson.
*/

/*
** Avoid the temptation to punt entirely to strftime;
** the output of strftime is supposed to be locale specific
** whereas the output of asctime is supposed to be constant.
*/

/*LINTLIBRARY*/

#include "private.h"
#include "tzfile.h"
//#include "thread_private.h"
#include "se_cdefs.h"

/*
** Some systems only handle "%.2d"; others only handle "%02d";
** "%02.2d" makes (most) everybody happy.
** At least some versions of gcc warn about the %02.2d;
** we conditionalize below to avoid the warning.
*/
/*
** All years associated with 32-bit time_t values are exactly four digits long;
** some years associated with 64-bit time_t values are not.
** Vintage programs are coded for years that are always four digits long
** and may assume that the newline always lands in the same place.
** For years that are less than four digits, we pad the output with
** leading zeroes to get the newline in the traditional place.
** The -4 ensures that we get four characters of output even if
** we call a strftime variant that produces fewer characters for some years.
** The ISO C 1999 and POSIX 1003.1-2004 standards prohibit padding the year,
** but many implementations pad anyway; most likely the standards are buggy.
*/
#ifdef __GNUC__
#define ASCTIME_FMT	"%.3s %.3s%3d %2.2d:%2.2d:%2.2d %-4s\n"
#else /* !defined __GNUC__ */
#define ASCTIME_FMT	"%.3s %.3s%3d %02.2d:%02.2d:%02.2d %-4s\n"
#endif /* !defined __GNUC__ */
/*
** For years that are more than four digits we put extra spaces before the year
** so that code trying to overwrite the newline won't end up overwriting
** a digit within a year and truncating the year (operating on the assumption
** that no output is better than wrong output).
*/
#ifdef __GNUC__
#define ASCTIME_FMT_B	"%.3s %.3s%3d %2.2d:%2.2d:%2.2d     %s\n"
#else /* !defined __GNUC__ */
#define ASCTIME_FMT_B	"%.3s %.3s%3d %02.2d:%02.2d:%02.2d     %s\n"
#endif /* !defined __GNUC__ */

#define STD_ASCTIME_BUF_SIZE	26
/*
** Big enough for something such as
** ??? ???-2147483648 -2147483648:-2147483648:-2147483648     -2147483648\n
** (two three-character abbreviations, five strings denoting integers,
** seven explicit spaces, two explicit colons, a newline,
** and a trailing ASCII nul).
** The values above are for systems where an int is 32 bits and are provided
** as an example; the define below calculates the maximum for the system at
** hand.
*/
#define MAX_ASCTIME_BUF_SIZE	(2*3+5*INT_STRLEN_MAXIMUM(int)+7+2+1+1)

static char *
asctime3(timeptr, buf, bufsize)
register const struct tm *	timeptr;
char *				buf;
int				bufsize;
{
	static const char	wday_name[][4] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static const char	mon_name[][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	register const char *	wn;
	register const char *	mn;
	char			year[INT_STRLEN_MAXIMUM(int) + 2];
	int			len;

	if (timeptr == NULL) {
		errno = EINVAL;
		strlcpy(buf, "??? ??? ?? ??:??:?? ????\n", bufsize);
		return buf;
	}
	if (timeptr->tm_wday < 0 || timeptr->tm_wday >= DAYSPERWEEK)
		wn = "???";
	else	wn = wday_name[timeptr->tm_wday];
	if (timeptr->tm_mon < 0 || timeptr->tm_mon >= MONSPERYEAR)
		mn = "???";
	else	mn = mon_name[timeptr->tm_mon];
	/*
	** Use strftime's %Y to generate the year, to avoid overflow problems
	** when computing timeptr->tm_year + TM_YEAR_BASE.
	** Assume that strftime is unaffected by other out-of-range members
	** (e.g., timeptr->tm_mday) when processing "%Y".
	*/
	(void) strftime(year, sizeof year, "%Y", timeptr);
	len = snprintf(buf, bufsize,
		((strlen(year) <= 4) ? ASCTIME_FMT : ASCTIME_FMT_B),
		wn, mn,
		timeptr->tm_mday, timeptr->tm_hour,
		timeptr->tm_min, timeptr->tm_sec,
		year);
	if (len != -1 && len < bufsize) {
		return buf;
	} else {
#ifdef EOVERFLOW
		errno = EOVERFLOW;
#else /* !defined EOVERFLOW */
		errno = EINVAL;
#endif /* !defined EOVERFLOW */
		return NULL;
	}
}

/*
** A la ISO/IEC 9945-1, ANSI/IEEE Std 1003.1, 2004 Edition.
*/

/*
 ** Enable asctime_r implementation at Windows platform for 
 ** sample code needs this implementation at Windows platform
 */
//#ifdef __GNUC__ /* Not for Windows */
char *
asctime_r(timeptr, buf)
register const struct tm *	timeptr;
char *				buf;
{
	/*
	** P1003 8.3.5.2 says that asctime_r() can only assume at most
	** a 26 byte buffer.
	*/
	return asctime3(timeptr, buf, STD_ASCTIME_BUF_SIZE);
}
//#endif

/*
** A la ISO/IEC 9945-1, ANSI/IEEE Std 1003.1, 2004 Edition.
*/

char *
asctime(timeptr)
const struct tm *	timeptr;
{
	static _TLIBC_THREAD_ char result[MAX_ASCTIME_BUF_SIZE];
	return asctime3(timeptr, result, (int)sizeof(result));
}
