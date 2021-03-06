//MySQL版
#include "jsongrab5.5.h"

extern MyQueue<weiboinfo>Qweiboinfo;
extern MyQueue<weibo>Qweibo;
extern interprocess_semaphore weibosemp;
extern interprocess_semaphore weibosemc;
extern bool IsExit;
extern MyLog statelog;

template <class T> std::string ConvertToString(T);
int itoa(int val, char* buf);

int jsongrabmysql(string filename)
{
	int i, count = 0;
	MYSQL mysql;

	/*
mysql_init (&mysql);
        if(!mysql_real_connect(&mysql,"localhost","root", NULL , "sina", 3306, NULL, 0))
        {
                cout << "mysql fail!" << endl;
        }
        else
                cout << "mysql connected!" << endl;
	*/

	std::ifstream infile(filename.c_str());
	Json::Value root;
	Json::Reader reader;
	string str, msg;
	if(infile.is_open())
	{
		while(infile>>str)
			msg += str;
	}
	int contentpos = 0, textpos = 0, quotpos = 0, picpos = 0;
	while(contentpos != -1)
	{
		contentpos = msg.find("content", contentpos + 1);
		cout << "contentpos:" << contentpos << endl;
		if(contentpos == -1)
			break;
		textpos = msg.find("text", contentpos);
		if(textpos == -1)
			break;
			cout << "textpos:" << textpos << msg[textpos] <<  endl;	
		
		quotpos = msg.find("\"", textpos + 7);
		cout << "quotpos:" << quotpos << endl;
		picpos = msg.find("pic", quotpos);
		cout << "picpos:" << picpos << endl;
		while(picpos - quotpos != 3)
		{
			try
			{
				msg.replace(quotpos, 1, " ");
				quotpos = msg.find("\"", quotpos + 1);
			}
			catch(out_of_range &oor)
			{
				cout << "out_of_range?" << endl;
				break;
				//
			}
			if(quotpos > picpos - 3)
				break;
						cout << "quotpos:" << quotpos << endl;
		}
		cout << endl;
	}
	cout << msg << endl;
//		fstream outfile(filenametmp.c_str(), ios::out);
//		outfile << msg;
//		outfile.close();


		//cout << msg << endl;
	bool  ok = reader.parse(msg, root);
    if (!ok)
    {
		//cout  << "Failed to parse configuration\n" << reader.getFormatedErrorMessages();
	//	statelog.write("fail file:%s\n", filename.c_str());
	//	statelog.flush();
		return 1;  
	}

	Json::Value arrayObj = root["result"];

	//	statelog.write("load file:%s\n", filename.c_str());
	//	statelog.flush();
	for(i = 0; i < arrayObj.size(); i++)
	{
	    struct weiboinfo weiboinfotmp;
	    struct weibo weibotmp;
	    string content, topic;
	    int firstsh, secondsh;
	    
	    weiboinfotmp.mblogid = arrayObj[i]["mblogid"].asString();
	    weiboinfotmp.basicinfo_uid = arrayObj[i]["uid"].asLargestInt();
	    weiboinfotmp.basicinfo_time = arrayObj[i]["time"].asLargestInt();
	    weiboinfotmp.basicinfo_source = arrayObj[i]["source"].asLargestInt();
	    weiboinfotmp.basicinfo_rtnum = arrayObj[i]["rtnum"].asInt();
	    weiboinfotmp.basicinfo_cmtnum = arrayObj[i]["cmtnum"].asInt();
	    weibotmp.content = arrayObj[i]["content"]["text"].asString();
	    content = weibotmp.content;
	    firstsh = content.find_first_of('#');
	    if(firstsh != -1)
	    {
			secondsh = content.find_first_of('#', firstsh + 1);
			int j;
			if(secondsh > firstsh && (secondsh - firstsh) < 20)
			{
				for(j = firstsh + 1; j != secondsh; j++)
					topic += content[j];
				weiboinfotmp.basicinfo_topic = topic;
				//cout << topic << endl;
			}
	    }

	    if(!arrayObj[i]["content"]["atUsers"].isObject())
		{
			if(!arrayObj[i]["content"]["atUsers"].empty())
			{
				Json::Value::Members member = (arrayObj[i]["content"]["atUsers"]).getMemberNames();
				vector<string>::iterator iMember;
				string atuser;
				//cout << *(member.begin()) << endl;
				for(iMember = member.begin(); iMember != member.end(); iMember++)
				{
					atuser += *iMember;
					if(iMember + 1 != member.end())
						atuser += ";";
				}
				weiboinfotmp.atusers = atuser;
			}
		}
	    
		if(!arrayObj[i]["rt"].isObject())
		{	
			
			if(!arrayObj[i]["rt"]["rootuid"].isObject())
				weiboinfotmp.rtinfo_rootuid = arrayObj[i]["rt"]["rootuid"].asLargestInt();	
			
			if(!arrayObj[i]["rt"]["fromuid"].isObject())
				weiboinfotmp.rtinfo_fromuid = arrayObj[i]["rt"]["fromuid"].asLargestInt();
			
			if(!arrayObj[i]["rt"]["rturl"].isObject())
			{
				string rturl = arrayObj[i]["rt"]["rturl"].asString();
				if(!rturl.empty())
					weiboinfotmp.rtinfo_fromid = rturl.substr(rturl.length() - 9, 9);
			}
			
			if(!arrayObj[i]["rt"]["rootid"].isObject())
				weiboinfotmp.rtinfo_rootid = arrayObj[i]["rt"]["rootid"].asString();
			
			if(!arrayObj[i]["rt"]["rootrtnum"].isObject())
			{
				weiboinfotmp.rtinfo_rootrtnum = 0;
				if(arrayObj[i]["rt"]["rootrtnum"].isString())
				{
					weiboinfotmp.rtinfo_rootrtnum =  atoi(arrayObj[i]["rt"]["rootrtnum"].asString().c_str());
				}
				else if(arrayObj[i]["rt"]["rootrtnum"].isInt())
					weiboinfotmp.rtinfo_rootrtnum = arrayObj[i]["rt"]["rootrtnum"].asLargestInt();
			}
			
			if(!arrayObj[i]["rt"]["fromuid"].isObject())
				weiboinfotmp.rtinfo_rtreason = arrayObj[i]["rt"]["rtreason"].asString();
	    
		}
		if(i == arrayObj.size() - 1)
		  {
		    weiboinfotmp.lastitem=1;
		    weiboinfotmp.filename=filename;
		  }
		else
		  {
		    weiboinfotmp.lastitem=0;
		  }

	//	weibosemp.wait();
	//	if(IsExit) 
	//		 break;
	//	Qweiboinfo.push(weiboinfotmp);
	 //   Qweibo.push(weibotmp);
	  //  weibosemc.post();
	    
	    infile.close();
	  }
	return 0;
}

template <class T> std::string ConvertToString(T value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int itoa(int val, char* buf)
{
	const unsigned int radix = 10;
	char* p;
	unsigned int a; //every digit
	int len;
	char* b; //start of the digit char
	char temp;
	unsigned int u;
	p = buf;

	if (val < 0)
	{
		*p++ = '-';
		val = 0 - val;
	}

	u = (unsigned int)val;
	b = p;
	do
	{
		a = u % radix;
		u /= radix;
		*p++ = a + '0';
	} while (u > 0);
	len = (int)(p - buf);
	*p-- = 0;
	do
	{
		temp = *p;
		*p = *b;
		*b = temp;
		--p;
		++b;
	}while (b < p);
	return len;
}
