#!/home/xulei/Downloads/Python-3.5.1/python
#!conding:utf-8

import json
import urllib.request
import urllib.parse

def gettime():
    url = 'http://api.k780.com/?app=life.time&appkey=10003&sign=b59bc3ef6191eb9f747dd4e83c99f2a4&format=json'
    response = urllib.request.urlopen(url)
    html = response.read().decode('utf-8')
    target= json.loads(html)
    #print(target['result']['datetime_2'])
    #print(target['result']['week_2'])
    output='''<h1 style="text-align:center">'''+target['result']['datetime_2']+" </h1>"
    output1='''<h1 style="text-align:center">'''+target['result']['week_2']+" </h1>"
    print (output,output1)
if __name__ == "__main__":
    gettime()
    
