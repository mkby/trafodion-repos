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

/* A demo program about how to write a simple Trafodion C UDF
   一个用来演示基本UDF写法的程序
   主要的要点：
   1.使用sqludr.h
   2.参数列表先定义所有的入口参数，在定义所有的出口参数，个数不限，每个参数都需要一个indicator表示是否为NULL。最后由SQLUDR_TRAIL_ARGS结束
   3.注意数据类型，
          SQLUDR_CHAR对应SQL的CHAR(n)
		  SQLUDR_VC_STRUCT对应SQL的VARCHAR
		  等等，具体需要查看sqludr.h文件
 */
 
#include "sqludr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

SQLUDR_LIBFUNC SQLUDR_INT32 regexpmatch(SQLUDR_VC_STRUCT *srcStr,    //input string
                                SQLUDR_CHAR *pattern,  	//the regular expression
                                SQLUDR_INT32 *out1,     //the output indicator, 0 is matching, 1 not match, -1 error
                                SQLUDR_INT16 *inInd1,
                                SQLUDR_INT16 *inInd2,
                                SQLUDR_INT16 *outInd1,
                                SQLUDR_TRAIL_ARGS)
{
    regex_t reg;
    regmatch_t pm[10];
    const size_t nmatch = 10;
    int cflags, z;
    cflags = REG_EXTENDED|REG_NEWLINE;
    
    if (calltype == SQLUDR_CALLTYPE_FINAL)
        return SQLUDR_SUCCESS;
    if (SQLUDR_GETNULLIND(inInd1) == SQLUDR_NULL ||
        SQLUDR_GETNULLIND(inInd2) == SQLUDR_NULL)
        (*out1) = -1;
    else
    {
        //remove pattern spaces
        int i;
        for(i = 0; i < strlen(pattern) ; i++)
        {
          if(pattern[i]==' ') pattern[i]=0;
        }
        z = regcomp(&reg, pattern, cflags);
        if (z != 0){
          (*out1) = -1;
	  regfree(&reg); 
	  return SQLUDR_ERROR;
        }
	z = regexec(&reg, srcStr->data, nmatch, pm, 0);
        if (z == REG_NOMATCH) 
	    (*out1) = 1;
		else
            (*out1) = 0;
    }
    regfree(&reg);
    return SQLUDR_SUCCESS;
}
/* HOW to use it?
如何使用？

Compile：
you must first setup a Trafodion dev enviorment, and source the env.sh before compile the UDF. 
It needs to access trafodion src code to compile.
必须先搭建一个Trafodion的开发环境，需要能访问trafodion的头文件和库，才能进行如下编译

gcc -g -Wall -fPIC -I$MY_SQROOT/export/include/sql -shared -o regexp.so regexp.c

Register：
直接拷贝下面的命令，在Linux Shell里面执行即可，将创建一个UDF，后面有一些使用的范例
C自带的regcomp好像不是特别好用，很多正则表达式的语法都貌似没有支持，这个例子只是一个范例
下面的例子从数据库中找出电话号码和邮件地址：

UDFLIB="'$(pwd)/regexp.so'"
sqlci <<EOF
create library myudfs file $UDFLIB;
drop function regexpmatch;
create function regexpmatch(varchar(100),char(100)) returns (regexpmatch int)
external name 'regexpmatch ' library myudfs
deterministic no sql no transaction required;
EOF

test:
测试：

sqlci <<EOF
create table t2 (c1 int not null, c2 varchar(100), c3 int, primary key(c1) );
insert into t2 values ( 1, 'myemail@esgyn.cn', 1);
insert into t2 values ( 2, '518-4891', 2);
select * from t2 where regexpmatch(c2, '[0-9][0-9][0-9]-[0-9][0-9][0-9][0-9]') = 0 ;
select * from t2 where regexpmatch(c2,'^\w+([-+.]\w+)*@\w+([-.]\w+)*.\w+([-.]\w+)*')=0;
EOF

*/
