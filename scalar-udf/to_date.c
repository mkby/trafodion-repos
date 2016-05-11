// @@@ START COPYRIGHT @@@
//
// Copyright (c) 2016, 易鲸捷, http://www.esgyn.cn.
//
// Licensed under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//
// @@@ END COPYRIGHT @@@

/* 
         to_date('2016-03-08', 'yyyy-MM-dd');
         to_date('20160308112134','yyyy-MM-dd hh24:mi:ss');
         to_date('2016/03/08 11:21:34','yyyy-MM-dd hh24:mi:ss');
        to_date('20160308112134','yyyy-MM-dd');
         to_date('2016-03-08 11:21:34','yyyy-MM-dd');
        to_date('20160308112134','yyyy-MM-dd');
         to_date('20160308','yyyy-MM-dd');


        to_char('2016-03-08 11:21:34','yyyy-MM-dd hh24:mi:ss');
        to_char('2016-03-08 11:21:34','yyyyMMdd');
        to_char('2016-03-08 11:21:34','yyyyMMddhh24miss');

 */
 
#include "sqludr.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

int replaceit(char *input, char *output, char* from, char* to)
{
	char *p = input;
	int flen = strlen(from);
	int tlen = strlen(to);
	int i = 0;
	int match = 0;
	int count = 0;
	
	while(*p)
	{
		match=0;
		if(p[0] == from[0])
		{
			match = 1;
			for(i = 0; i< flen ; i++)
			{
				if(p[i] == from[i])
					continue;
				else 
				{
					match=0;
					break; //not match
				}
			}
			if(match == 1)
			{
				//do replace
				strcat(output,to);
				p+=flen;
				count++;
			}
		}
		if(match == 0)	
		{
			int curLen = strlen(output);
			output[curLen] = p[0]; //append
			output[curLen+1] = 0; 
			p++;
		}
	}
	return count;
}
int convertFormatString(char * sqlformat, char *glibcformat)
{
	int isDate = 0;  //date
	int ret=0;
	char tmpbuf[128];
	memset(tmpbuf ,  0, 128);
	strcpy(tmpbuf,sqlformat);
	memset(glibcformat, 0, sizeof(glibcformat));
	//yyyy to 
	replaceit(tmpbuf,glibcformat,"yyyy","%Y");
	//MM
	strcpy(tmpbuf,glibcformat);
	memset(glibcformat,0,sizeof(glibcformat));
	replaceit(tmpbuf,glibcformat,"MM","%m");
	//dd to %d
	strcpy(tmpbuf,glibcformat);
	memset(glibcformat,0,sizeof(glibcformat));
	replaceit(tmpbuf,glibcformat,"dd","%d");
	
	//hh24
	strcpy(tmpbuf,glibcformat);
	memset(glibcformat,0,sizeof(glibcformat));
	ret= replaceit(tmpbuf,glibcformat,"hh24","%H");
	isDate+=ret;
	//mi
	strcpy(tmpbuf,glibcformat);
	memset(glibcformat,0,sizeof(glibcformat));
	ret= replaceit(tmpbuf,glibcformat,"mi","%M");
	isDate+=ret;
	//ss
	strcpy(tmpbuf,glibcformat);
	memset(glibcformat,0,sizeof(glibcformat));
	ret = replaceit(tmpbuf,glibcformat,"ss","%S");
	isDate+=ret;
	return isDate ;
}
SQLUDR_LIBFUNC SQLUDR_INT32 tsudf(SQLUDR_CHAR *in,
                                  SQLUDR_CHAR *out,
                                  SQLUDR_INT16 *inInd,
                                  SQLUDR_INT16 *outInd,
                                  SQLUDR_TRAIL_ARGS)
{
  if (calltype == SQLUDR_CALLTYPE_FINAL)
    return SQLUDR_SUCCESS;

  if (*inInd == SQLUDR_NULL)
  {
    *outInd = SQLUDR_NULL;
  }
  else
  {

    SQLUDR_UINT32 inLen = 19;

    memcpy(out, in, inLen-2);
    memcpy(&out[inLen-2], "00", 2);
  }

  return SQLUDR_SUCCESS;
}

SQLUDR_LIBFUNC SQLUDR_INT32 tochar(SQLUDR_CHAR *srcStr,    //input string
                                SQLUDR_CHAR *pattern,  	//the regular expression
                                SQLUDR_CHAR *out1,     //the output indicator, 0 is matching, 1 not match, -1 error
                                SQLUDR_INT16 *inInd1,
                                SQLUDR_INT16 *inInd2,
                                SQLUDR_INT16 *outInd1,
                                SQLUDR_TRAIL_ARGS)
{
    struct tm the_tm;
	char buf[255];
	char cpattern[128];
	memset(cpattern,0,128);
	char inputStr[128];
	memset(inputStr,0,128);
	int isDate = 0;
	
    if (calltype == SQLUDR_CALLTYPE_FINAL)
        return SQLUDR_SUCCESS;
    if (SQLUDR_GETNULLIND(inInd1) == SQLUDR_NULL ||
        SQLUDR_GETNULLIND(inInd2) == SQLUDR_NULL)
    {
         SQLUDR_SETNULLIND(outInd1);
         return SQLUDR_SUCCESS;
    }
    else
    {
        int i;
        for(i = strlen(pattern)-1; i >0; i--)
        {
          if(pattern[i]==' ') pattern[i]=0;
          else if(pattern[i]=='\n') pattern[i]=0;
	  else
		break;
        }
	memcpy(inputStr,srcStr,19);
		isDate = convertFormatString(pattern, cpattern);
		strptime((const char*)srcStr,"%Y-%m-%d %H:%M:%S",&the_tm);
	strftime(buf, sizeof(buf), cpattern, &the_tm);

	strcpy(out1,buf);

    }
	
    return SQLUDR_SUCCESS;
}


SQLUDR_LIBFUNC SQLUDR_INT32 formattimedate(SQLUDR_VC_STRUCT *srcStr,    //input string
                                SQLUDR_CHAR *pattern,  	//the regular expression
                                SQLUDR_CHAR *out1,     //the output indicator, 0 is matching, 1 not match, -1 error
                                SQLUDR_INT16 *inInd1,
                                SQLUDR_INT16 *inInd2,
                                SQLUDR_INT16 *outInd1,
                                SQLUDR_TRAIL_ARGS)
{
    struct tm the_tm;
	char buf[255];
	char cpattern[128];
	memset(cpattern,0,128);
	int isDate = 0;
	
    if (calltype == SQLUDR_CALLTYPE_FINAL)
        return SQLUDR_SUCCESS;
    if (SQLUDR_GETNULLIND(inInd1) == SQLUDR_NULL ||
        SQLUDR_GETNULLIND(inInd2) == SQLUDR_NULL)
         return SQLUDR_SUCCESS;
    else
    {
        int i;
        for(i = strlen(pattern)-1; i >0; i--)
        {
          if(pattern[i]==' ') pattern[i]=0;
          else if(pattern[i]=='\n') pattern[i]=0;
	  else
		break;
        }
	isDate = convertFormatString(pattern, cpattern);
	
	//srcStr->data[srcStr->length]=0;
	the_tm.tm_sec = 0;
	the_tm.tm_min = 0;
	the_tm.tm_hour = 0;
	the_tm.tm_mday = 0;
	the_tm.tm_mon = 0;
	the_tm.tm_year = 0;

	strptime((const char*)srcStr->data,(const char*)cpattern,&the_tm);
	if(isDate == 0)
		strftime(buf,sizeof(buf), "%Y-%m-%d", &the_tm);
	else
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &the_tm);
	if(isDate !=0)
	{
		memcpy(out1,buf,19);
	}
	else
	{
		memcpy(out1,buf,10);
		int ii=0;
		for(ii=0;ii<9;ii++) out1[ii+10]=0x20;
	}
	//out1->length = 19; // strlen(buf);
	//memcpy(out1->data, buf, strlen(buf));
    }
	
    return SQLUDR_SUCCESS;
}
/* HOW to use it?
如何使用？

Compile：
you must first setup a Trafodion dev enviorment, and source the env.sh before compile the UDF. 
It needs to access trafodion src code to compile.
必须先搭建一个Trafodion的开发环境，需要能访问trafodion的头文件和库，才能进行如下编译

gcc -g -fPIC -I$MY_SQROOT/export/include/sql -shared -o to_char.so to_date.c

Register：
直接拷贝下面的命令，在Linux Shell里面执行即可，将创建一个UDF，后面有一些使用的范例
C自带的regcomp好像不是特别好用，很多正则表达式的语法都貌似没有支持，这个例子只是一个范例
下面的例子从数据库中找出电话号码和邮件地址：

UDFLIB="'$(pwd)/to_char.so'"
sqlci <<EOF
create library myudfs file $UDFLIB;
drop function tochar;
create function tochar(timestamp,char(128)) returns char(128) 
external name 'tochar' library myudfs
deterministic no sql no transaction required;
create function formattime(varchar(128),char(128)) returns  (formattime char(128))
external name 'formattimedate' library MYUDFSTOCHAR
deterministic no sql no transaction required;
EOF

test:
测试：

sqlci <<EOF
create table t2 (c1 int not null, c2 timestamp, c3 date, primary key(c1) );
insert into t2 values ( 1, timestamp'1985-01-25 10:10:10', date'1985-01-25');
select c1, to_char(c2), to_char(c3),to_char('2016-03-08 11:21:34','yyyy-MM-dd hh24:mi:ss'), to_char('20160308','yyyyMMdd') from t2 ;
EOF

*/
