/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   时间
*************************************************************************/

/* 
 * 时间操作 c++11可以使用std::chrono
*/

#ifndef _DATETIME_H_
#define _DATETIME_H_

#ifdef _MSC_VER
#include <time.h>
#include <windows.h> 
#else 
#include <sys/time.h>
#endif

#include <string>

static char DEFAULT_DATETIME_FORMAT[50]= "%Y-%m-%d %H:%M:%S";

class CTime
{
public:
    //window上实现strptime函数，linux已经提供strptime
    static char *strptime(const char *buf, const char *fmt, struct tm *tm);

	// 获取当天凌晨的时间戳（s）
	static int64_t getTodayBeginTime();
	// 获取当天结束的时间戳（s）
	static int64_t getTodayEndTime();
	// 获取当天剩余时间（s）
	static int32_t getTodayLeftSecond();
    // 获取当前时间的GMT时间
    static std::string getGmtTime();
};

class CDuration
{
public:
    /* @brief  构造函数，时间间隔
     * @param nDay    间隔天数
     * @param nHour   间隔小时
     * @param nMinute 间隔分钟
     * @param nSecond 间隔秒
    */
    CDuration(uint32_t nDay, uint32_t nHour, uint32_t nMinute, uint32_t nSecond);

    time_t getTimeSec() { return m_TimeSec; }
private:
	time_t m_TimeSec;
};

class CDatetime
{
public:    
    /*
    * @brief 构造函数, 精度毫秒
    */
    CDatetime();

    /*
    * @brief 构造函数
    * @param sDate 时间 (当不含时分秒时, bFlag为true, 如"2018-03-02")
    */
    CDatetime(const std::string &sDate, const std::string &sFormat, bool bFlag = false);
    // 精度秒
    CDatetime(const time_t nTime);

    // 刷新当前时间
    void flushTime();

    // 获取年月日时分秒等接口...
    int getYear() const;
    int getMonth() const;
    int getDay() const;
    int getHour() const;
    int getMin() const;

    int getYday() const;

    // 获取秒数
    time_t getSec() const;

    // 设置秒数
    void setSec(time_t nSec);

    // 获取当前时间毫秒数
    long long getMillisecond() const;

    // 获取星期几
    // 0~6 0-星期天 1-星期一 ...
    int getDayOfWeek() const;

    // 获取获取星期几
    // 星期天~星期六
    std::string getWeekday() const;

    // 获取当前时间已过多少秒
    int getCurTimePassSec();

    /*
    * @brief: 
    * @param format 字符串格式          如："%Y-%m-%d %H:%M%S"
    * @return       返回字符串格式的时间 如: 2018-02-08 08:48:14 
    */
    std::string getStrTime(const char *format = DEFAULT_DATETIME_FORMAT) const;

    time_t operator-(const CDatetime &Right);
    bool operator==(const CDatetime &Right);

    CDatetime& operator=(time_t nSec);
    CDatetime& operator=(CDatetime &t);
    CDatetime& operator+=(CDuration &duration);
    CDatetime& operator-=(CDuration &duration);

    // 给一个时间点加/减一个时间间隔, 生成一个新的CDatetime类型时间点
    CDatetime operator-(CDuration &duration);
    CDatetime operator+(CDuration &duration);
    CDatetime add(CDuration &duration);
    CDatetime sub(CDuration &duration);

private:
	//当前时间的秒数
	time_t m_TimeSec;

	//当前时间的毫秒数，1970-1-1日到当前的总毫秒数
	long long m_TimesMillisec;
};

#endif /* _DATETIME_H_ */