/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   时间
*************************************************************************/
#include <string.h>
#include "datetime.h"

#define TM_YEAR_BASE 1900
 
/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E     0x01
#define ALT_O     0x02
#define LEGAL_ALT(x)    { if (alt_format & ~(x)) return (0); }

static const char *day[7] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
  "Friday", "Saturday"
};
static const char *abday[7] = {
  "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static const char *mon[12] = {
  "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December"
};
static const char *abmon[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *am_pm[2] = {
  "AM", "PM"
};

static const char *WEEKDAY[7] = {"Sunday", "Monday", "Tuesday", "Wednesday","Thursday", "Friday", "Saturday"};

static int conv_num(const char **buf, int *dest, int llim, int ulim)
{
    int result = 0;

    /* The limit also determines the number of valid digits. */
    int rulim = ulim;

    if (**buf < '0' || **buf > '9')
        return (0);

    do {
        result *= 10;
        result += *(*buf)++ - '0';
        rulim /= 10;
    } while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

    if (result < llim || result > ulim)
        return (0);

    *dest = result;
    return (1);
}

char *CTime::strptime(const char *buf, const char *fmt, struct tm *tm)
{
    char c;
    const char *bp;
    size_t len = 0;
    int alt_format, i, split_year = 0;
 
    bp = buf;
 
    while ((c = *fmt) != '\0') {
        /* Clear `alternate' modifier prior to new conversion. */
        alt_format = 0;
 
        /* Eat up white-space. */
        if (isspace(c)) {
            while (isspace(*bp))
                bp++;

            fmt++;
            continue;
        }
            
        if ((c = *fmt++) != '%')
            goto literal;

again:  switch (c = *fmt++) {
        case '%': /* "%%" is converted to "%". */
literal:
            if (c != *bp++)
                return (0);
            break;
    
        /*
        * "Alternative" modifiers. Just set the appropriate flag
        * and start over again.
        */
        case 'E': /* "%E?" alternative conversion modifier. */
            LEGAL_ALT(0);
            alt_format |= ALT_E;
            goto again;
    
        case 'O': /* "%O?" alternative conversion modifier. */
            LEGAL_ALT(0);
            alt_format |= ALT_O;
            goto again;
        
        /*
        * "Complex" conversion rules, implemented through recursion.
        */
        case 'c': /* Date and time, using the locale's format. */
            LEGAL_ALT(ALT_E);
            if (!(bp = strptime(bp, "%x %X", tm)))
                return (0);
            break;
    
        case 'D': /* The date as "%m/%d/%y". */
            LEGAL_ALT(0);
            if (!(bp = strptime(bp, "%m/%d/%y", tm)))
                return (0);
            break;
 
        case 'R': /* The time as "%H:%M". */
            LEGAL_ALT(0);
            if (!(bp = strptime(bp, "%H:%M", tm)))
                return (0);
            break;
    
        case 'r': /* The time in 12-hour clock representation. */
            LEGAL_ALT(0);
            if (!(bp = strptime(bp, "%I:%M:%S %p", tm)))
                return (0);
            break;
 
        case 'T': /* The time as "%H:%M:%S". */
            LEGAL_ALT(0);
            if (!(bp = strptime(bp, "%H:%M:%S", tm)))
                return (0);
            break;
 
        case 'X': /* The time, using the locale's format. */
            LEGAL_ALT(ALT_E);
            if (!(bp = strptime(bp, "%H:%M:%S", tm)))
                return (0);
            break;
 
        case 'x': /* The date, using the locale's format. */
            LEGAL_ALT(ALT_E);
            if (!(bp = strptime(bp, "%m/%d/%y", tm)))
                return (0);
            break;
 
        /*
        * "Elementary" conversion rules.
        */
        case 'A': /* The day of week, using the locale's form. */
        case 'a':
            LEGAL_ALT(0);
            for (i = 0; i < 7; i++) {
                /* Full name. */
                len = strlen(day[i]);
                if (strncmp(day[i], bp, len) == 0)
                    break;

                /* Abbreviated name. */
                len = strlen(abday[i]);
                if (strncmp(abday[i], bp, len) == 0)
                    break;
            }
    
            /* Nothing matched. */
            if (i == 7)
                return (0);
        
            tm->tm_wday = i;
            bp += len;
            break;
 
        case 'B': /* The month, using the locale's form. */
        case 'b':
        case 'h':
            LEGAL_ALT(0);
            for (i = 0; i < 12; i++) {
                /* Full name. */
                len = strlen(mon[i]);
                if (strncmp(mon[i], bp, len) == 0)
                    break;
        
                /* Abbreviated name. */
                len = strlen(abmon[i]);
                if (strncmp(abmon[i], bp, len) == 0)
                    break;
            }
        
            /* Nothing matched. */
            if (i == 12)
                return (0);
        
            tm->tm_mon = i;
            bp += len;
            break;
    
        case 'C': /* The century number. */
            LEGAL_ALT(ALT_E);
            if (!(conv_num(&bp, &i, 0, 99)))
                return (0);
        
            if (split_year) {
                tm->tm_year = (tm->tm_year % 100) + (i * 100);
            } else {
                tm->tm_year = i * 100;
                split_year = 1;
            }
            break;
    
        case 'd': /* The day of month. */
        case 'e':
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
                return (0);
            break;
    
        case 'k': /* The hour (24-hour clock representation). */
            LEGAL_ALT(0);
        /* FALLTHROUGH */
        case 'H':
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
                return (0);
            break;
    
        case 'l': /* The hour (12-hour clock representation). */
            LEGAL_ALT(0);
        /* FALLTHROUGH */
        case 'I':
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
                return (0);
            if (tm->tm_hour == 12)
                tm->tm_hour = 0;
            break;
 
        case 'j': /* The day of year. */
            LEGAL_ALT(0);
            if (!(conv_num(&bp, &i, 1, 366)))
                return (0);
            tm->tm_yday = i - 1;
            break;
    
        case 'M': /* The minute. */
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
                return (0);
            break;
    
        case 'm': /* The month. */
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &i, 1, 12)))
                return (0);
            tm->tm_mon = i - 1;
            break;
    
        case 'p': /* The locale's equivalent of AM/PM. */
            LEGAL_ALT(0);
            /* AM? */
            if (strcmp(am_pm[0], bp) == 0) {
                if (tm->tm_hour > 11)
                    return (0);
        
                bp += strlen(am_pm[0]);
                break;
            }
            /* PM? */
            else if (strcmp(am_pm[1], bp) == 0) {
                if (tm->tm_hour > 11)
                    return (0);
        
                tm->tm_hour += 12;
                bp += strlen(am_pm[1]);
                break;
            }
    
            /* Nothing matched. */
            return (0);
 
        case 'S': /* The seconds. */
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
                return (0);
            break;
 
        case 'U': /* The week of year, beginning on sunday. */
        case 'W': /* The week of year, beginning on monday. */
            LEGAL_ALT(ALT_O);
            /*
            * XXX This is bogus, as we can not assume any valid
            * information present in the tm structure at this
            * point to calculate a real value, so just check the
            * range for now.
            */
            if (!(conv_num(&bp, &i, 0, 53)))
                return (0);
            break;
 
        case 'w': /* The day of week, beginning on sunday. */
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
                return (0);
            break;
 
        case 'Y': /* The year. */
            LEGAL_ALT(ALT_E);
            if (!(conv_num(&bp, &i, 0, 9999)))
                return (0);
        
            tm->tm_year = i - TM_YEAR_BASE;
            break;
 
        case 'y': /* The year within 100 years of the epoch. */
            LEGAL_ALT(ALT_E | ALT_O);
            if (!(conv_num(&bp, &i, 0, 99)))
                return (0);
        
            if (split_year) {
                tm->tm_year = ((tm->tm_year / 100) * 100) + i;
                break;
            }
            split_year = 1;
            if (i <= 68)
                tm->tm_year = i + 2000 - TM_YEAR_BASE;
            else
                tm->tm_year = i + 1900 - TM_YEAR_BASE;
            break;
 
        /*
        * Miscellaneous conversions.
        */
        case 'n': /* Any kind of white-space. */
        case 't':
            LEGAL_ALT(0);
            while (isspace(*bp))
                bp++;
            break;
 
        default:  /* Unknown/unsupported conversion. */
            return (0);
        }
    }
    /* LINTED functional specification */
    return ((char *)bp);
}

int64_t CTime::getTodayBeginTime()
{
    time_t now = time(NULL);
#ifdef _MSC_VER	
    struct tm today = *localtime(&now);
#else
    struct tm today = { 0 };
    localtime_r(&now, &today);
#endif

    today.tm_hour = 0; today.tm_min = 0; today.tm_sec = 0;

    return mktime(&today);
}

int64_t CTime::getTodayEndTime()
{
    return getTodayBeginTime() + 24 * 3600;
}

int32_t CTime::getTodayLeftSecond()
{
    return (int32_t)(getTodayEndTime() - time(NULL));
}

std::string CTime::getGmtTime() {
	time_t now = time(NULL);
	struct tm* gmt = gmtime(&now);

	const char* fmt = "%a, %d %b %Y %H:%M:%S GMT";
	char tstr[30];

	strftime(tstr, sizeof(tstr), fmt, gmt);

	return tstr;
}

CDuration::CDuration(uint32_t nDay, uint32_t nHour, uint32_t nMinute, uint32_t nSecond)
{
    m_TimeSec = 0;
    int sec = nSecond + nMinute * 60 + nHour * 60 * 60 + nDay * 24 * 60 * 60;
    m_TimeSec += sec;
}

CDatetime::CDatetime()
{
#ifdef _MSC_VER
	SYSTEMTIME current;
	GetLocalTime(&current);
	struct tm ltm;
	memset(&ltm, 0, sizeof(ltm));
	ltm.tm_year = current.wYear - 1900;
	ltm.tm_mon = current.wMonth - 1;
	ltm.tm_mday = current.wDay;
	ltm.tm_hour = current.wHour;
	ltm.tm_min = current.wMinute;
	ltm.tm_sec = current.wSecond;
	m_TimeSec = mktime(&ltm);
	//uint64_t temptime = m_TimeSec;
	m_TimesMillisec = m_TimeSec * 1000 + current.wMilliseconds;
#else
    struct timeval current;
    gettimeofday(&current, NULL);
    m_TimeSec = current.tv_sec;
    m_TimesMillisec = m_TimeSec * 1000 + current.tv_usec / 1000;
#endif
}

CDatetime::CDatetime(const std::string &sDate, const std::string &sFormat, bool bFlag)
{
	struct tm stm;
#ifdef _MSC_VER	
    CTime::strptime(sDate.c_str(), sFormat.c_str(), &stm);
#else
    strptime(sDate.c_str(), sFormat.c_str(), &stm);
#endif

    if(bFlag){
        stm.tm_min = 0;
	    stm.tm_sec = 0;
	    stm.tm_hour = 0;
    }
    
    m_TimeSec = mktime(&stm);
    m_TimesMillisec = m_TimeSec * 1000;
}

CDatetime::CDatetime(const time_t nTime)
{
    m_TimeSec = nTime;

    m_TimesMillisec = m_TimeSec * 1000;
}

CDatetime::CDatetime(const CDatetime &right) 
{
    m_TimeSec = right.m_TimeSec;
    m_TimesMillisec = right.m_TimesMillisec;
}

int CDatetime::getYear() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
    return ptm->tm_year + 1900;
#else
    struct tm ptm = { 0 };
    localtime_r(&m_TimeSec, &ptm);
    return ptm.tm_year + 1900;
#endif    
}

int CDatetime::getMonth() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
    return ptm->tm_mon + 1; 
#else
    struct tm ptm = { 0 };
    localtime_r(&m_TimeSec, &ptm);
    return ptm.tm_mon + 1; 
#endif
}

int CDatetime::getDay() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
    return ptm->tm_mday;
#else
    struct tm ptm = { 0 };
    localtime_r(&m_TimeSec, &ptm);
    return ptm.tm_mday;
#endif
}

int CDatetime::getHour() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
    return ptm->tm_hour;
#else
    struct tm ptm = { 0 };
    localtime_r(&m_TimeSec, &ptm);
    return ptm.tm_hour; 
#endif 
}

int CDatetime::getMinute() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
    return ptm->tm_min;
#else
    struct tm ptm = { 0 };
    localtime_r(&m_TimeSec, &ptm);
    return ptm.tm_min;
#endif
}

int CDatetime::getSecond() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
    return ptm->tm_sec;
#else
    struct tm ptm = { 0 };
    localtime_r(&m_TimeSec, &ptm);
    return ptm.tm_sec;
#endif
}

int CDatetime::getYday() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
    return ptm->tm_yday;
#else
    struct tm ptm = { 0 };
    localtime_r(&m_TimeSec, &ptm);
    return ptm.tm_yday;
#endif
}

int CDatetime::getDayOfWeek() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
    return ptm->tm_wday;
#else
    struct tm ptm = { 0 };
    localtime_r(&m_TimeSec, &ptm);
    return ptm.tm_wday;
#endif
}

std::string CDatetime::getWeekday() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
#else
    struct tm stuTm = { 0 };
    localtime_r(&m_TimeSec, &stuTm);
    struct tm *ptm = &stuTm;
#endif

    int tm_wday = ptm->tm_wday;
    
    return WEEKDAY[tm_wday];
}

int CDatetime::getWeekOfYear() const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
#else
    struct tm stuTm = { 0 };
    localtime_r(&m_TimeSec, &stuTm);
    struct tm *ptm = &stuTm;
#endif

    int tm_wday = ptm->tm_wday;
    int tm_yday = ptm->tm_yday;

    // 本年1月1日是周几
    int baseWeekDay = 7 - (tm_yday + 1 - (tm_wday + 1)) % 7; 
    if (baseWeekDay == 7)
        baseWeekDay = 0; //0代表周日, 一周的开始
    // 本周是一年的第几周
    return (baseWeekDay + tm_yday) / 7 + 1;
}

std::string CDatetime::getStrTime(const char *format) const
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
#else
    struct tm stuTm = { 0 };
    localtime_r(&m_TimeSec, &stuTm);
    struct tm *ptm = &stuTm;
#endif

	char nowtime[128] = {0};
	strftime(nowtime, 128, format, ptm);
	return nowtime;
}

time_t CDatetime::operator-(const CDatetime &right)
{
    return this->m_TimesMillisec - right.m_TimesMillisec;
}

bool CDatetime::operator==(const CDatetime &right)
{
    return this->m_TimesMillisec == right.m_TimesMillisec;
}

CDatetime& CDatetime::operator=(CDatetime &t)
{
    if(this == &t)
        return *this;
    
    m_TimeSec = t.m_TimeSec;
    m_TimesMillisec = t.m_TimeSec * 1000;

    return *this;
}

CDatetime& CDatetime::operator=(time_t nSec)
{
    m_TimeSec = nSec;
    m_TimesMillisec = nSec * 1000;

    return *this;
}

CDatetime CDatetime::operator-(CDuration &duration)
{
    CDatetime tmp(*this);
    tmp -= duration;
    return tmp;
}

CDatetime CDatetime::operator+(CDuration &duration)
{
    CDatetime tmp(*this);
    tmp += duration;
    return tmp;
}

CDatetime CDatetime::add(CDuration &duration)
{
    *this += duration;
    return *this;
}

CDatetime CDatetime::sub(CDuration &duration)
{
    *this -= duration;
    return *this;
}

CDatetime& CDatetime::operator+=(CDuration &duration)
{
    m_TimeSec += duration.getTimeSec();
    m_TimesMillisec = m_TimeSec * 1000;
    return *this;
}

CDatetime& CDatetime::operator-=(CDuration &duration)
{
    m_TimeSec -= duration.getTimeSec();
    m_TimesMillisec = m_TimeSec * 1000;
    return *this;
}

void CDatetime::flushTime()
{
#ifdef _MSC_VER
	SYSTEMTIME current;
	GetLocalTime(&current);
	struct tm ltm;
	memset(&ltm, 0, sizeof(ltm));
	ltm.tm_year = current.wYear - 1900;
	ltm.tm_mon = current.wMonth - 1;
	ltm.tm_mday = current.wDay;
	ltm.tm_hour = current.wHour;
	ltm.tm_min = current.wMinute;
	ltm.tm_sec = current.wSecond;
	m_TimeSec = mktime(&ltm);
	//uint64_t temptime = m_TimeSec;
	m_TimesMillisec = m_TimeSec * 1000 + current.wMilliseconds;
#else
    struct timeval current;
    gettimeofday(&current, NULL);
    m_TimeSec = current.tv_sec;
    m_TimesMillisec = m_TimeSec * 1000 + current.tv_usec / 1000;
#endif
}

int CDatetime::getCurTimePassSec()
{
#ifdef _MSC_VER	
    struct tm *ptm = localtime(&m_TimeSec);
#else
    struct tm stuTm = { 0 };
    localtime_r(&m_TimeSec, &stuTm);
    struct tm *ptm = &stuTm;
#endif

    int hour = ptm->tm_hour;
    int minute = ptm->tm_min;
    int second = ptm->tm_sec;

    return hour * 3600 + minute * 60 + second;
}

time_t CDatetime::getSec() const
{
    return m_TimeSec;
}

long long CDatetime::getMillisecond() const
{
    return m_TimesMillisec;
}

void CDatetime::setSec(time_t nSec)
{
    m_TimeSec = nSec;

    m_TimesMillisec = m_TimeSec * 1000;
}

time_t CDatetime::getTimestamp() const
{
    return m_TimeSec;
}
void CDatetime::setTimestamp(time_t nSec)
{
    m_TimeSec = nSec;

    m_TimesMillisec = m_TimeSec * 1000;
}