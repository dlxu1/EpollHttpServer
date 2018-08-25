#!/usr/bin/python
#!coding:utf8

import os
def deal_with():
	str="0"
	if cmp(os.getenv("math"),"GET")==0:
	    str=os.getenv("query_string")
	#print str
	sub1,sub2= str.split("&")
	sub1=sub1[2:]
	sub2=sub2[2:]
	print "计算结果：" 
	print sub1+'*'+sub2+"=%d"%(int(sub2)*int(sub1))

if __name__ == "__main__":
	deal_with()
