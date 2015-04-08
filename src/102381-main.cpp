#include <iostream>
#include <windows.h>
using namespace std;

typedef unsigned short KeyType[5];
typedef unsigned short BlockType;
KeyType Key;

BlockType SBox_Encode(BlockType x)
{//S盒置换
    BlockType y=0;
    unsigned short TTemp=x,Temp[4],S[16]={14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7};
	for(int i=4;i>0;i--){Temp[i-1]=TTemp&15;TTemp>>=4;}
    for(i=0;i<3;i++){TTemp+=S[Temp[i]];TTemp<<=4;}TTemp+=S[Temp[i]];
	y=TTemp;
	return y;
}

BlockType SBox_Decode(BlockType x)
{//S盒逆变换
    BlockType y=0;
    unsigned short TTemp=x,Temp[4],S[16]={14,3,4,8,1,12,10,15,7,13,9,6,11,2,0,5};
	for(int i=4;i>0;i--){Temp[i-1]=TTemp&15;TTemp>>=4;}
    for(i=0;i<3;i++){TTemp+=S[Temp[i]];TTemp<<=4;}TTemp+=S[Temp[i]];
	y=TTemp;
	return y;
}

BlockType PBox_Encode(BlockType x)
{//P盒置换
	unsigned short c,P[16]={1,5,9,13,2,6,10,14,3,7,11,15,4,8,12,16};
	BlockType y=0;
	for(int i=0;i<15;i++)
	{
		c= x & (0x8000 >> (P[i]-1) );
		if (c>0) y=y+1;
		y<<=1;
	}
		c= x & (0x8000 >> (P[i]-1) );
		if (c>0) y=y+1;
	return y;
}

BlockType PBox_Decode(BlockType x)
{//P盒逆变换
	unsigned short c,P[16]={1,5,9,13,2,6,10,14,3,7,11,15,4,8,12,16};
	BlockType y=0;
	for(int i=0;i<15;i++)
	{
		c= x & (0x8000 >> (P[i]-1) );
		if (c>0) y=y+1;
		y<<=1;
	}
		c= x & (0x8000 >> (P[i]-1) );
		if (c>0) y=y+1;
	return y;
}

void OutPut_Bin(BlockType p)
{//输出显示一个16位二进制数
	unsigned short a[16];
    for(int i=0;i<16;i++){a[i]=(p%2);p/=2;}
	for(i=16;i>0;i--)cout<<a[i-1];
}

void OutPut_Key(KeyType key)
{//输出显示一个密钥
	for(int i=0;i<5;i++){OutPut_Bin(key[i]);cout<<endl;}
}

void BlockEncryption(BlockType PlainText,BlockType &CipherText,KeyType Key)
{//分组加密函数
	BlockType Temp=PlainText;
	for(int i=0;i<3;i++)
	{
        Temp^=Key[i];
		Temp=SBox_Encode(Temp);
		Temp=PBox_Encode(Temp);
	}
	Temp^=Key[i++];
    Temp=SBox_Encode(Temp);
    Temp^=Key[i];
	CipherText=Temp;
}

void BlockDecryption(BlockType &PlainText,BlockType CipherText,KeyType Key)
{//分组解密函数
	BlockType Temp=CipherText;
	int i=4;
	Temp^=Key[i--];
    Temp=SBox_Decode(Temp);
	for(;i>0;i--)
	{
        Temp^=Key[i];
		Temp=PBox_Decode(Temp);
		Temp=SBox_Decode(Temp);
	}
    Temp^=Key[i];
	PlainText=Temp;
}

void Key_Engine()
{//生成指定密钥
	unsigned int Temp=0x3a94d63f;
	for(int i=5;i>0;i--){Key[i-1]=Temp;Temp>>=4;}
}

void Key_Random()
{//随机生成密钥
    srand(GetTickCount());
	for(int i=5;i>0;i--)Key[i-1]=rand()%0x10000;
}

int FileEncryption(char *PlainFile,char *CipherFile,KeyType Key)
{//文件加密
	int i,n=0;
	BlockType Plain,Cipher,y=0;
	unsigned char w=0,v=0,Buf;
	FILE *PStream,*CStream,*Temp;
	PStream=fopen(PlainFile,"rb");
	if(!PStream){cout<<"文件 "<<PlainFile<<" 不存在!!!"<<endl;return 1;}
	Temp=fopen("Temp1.cip","wb");
	w=strlen(PlainFile);
	fwrite(&v,1,1,Temp);
	fwrite(&w,1,1,Temp);
	for(i=0;i<w;i++)
	{
		CipherFile[i]=PlainFile[i];
		fwrite(&PlainFile[i],1,1,Temp);
	}
	CipherFile[i++]='.';CipherFile[i++]='c';
	CipherFile[i++]='i';CipherFile[i++]='p';CipherFile[i]=0;
	fread(&Buf,1,1,PStream);
	while(!feof(PStream))
	{
        fwrite(&Buf,1,1,Temp);
        fread(&Buf,1,1,PStream);
		n++;
	}
	v=(n+w)%2;
	if(v)fwrite(&Buf,1,1,Temp);
	fclose(PStream);
	fclose(Temp);

        ///////*****    Temp	*****/////// 

	Temp=fopen("Temp1.cip","rb");
	if(!Temp){cout<<"错误!!!"<<endl;return 1;}
	CStream=fopen(CipherFile,"wb");
	fread(&Plain,2,1,Temp);
	Plain+=v;
	Plain^=y;
	BlockEncryption(Plain,Cipher,Key);
	y=Cipher;
	fwrite(&Cipher,2,1,CStream);
	fread(&Plain,2,1,Temp);
	while(!feof(Temp))
	{
		Plain^=y;
		BlockEncryption(Plain,Cipher,Key);
		y=Cipher;
		fwrite(&Cipher,2,1,CStream);
		fread(&Plain,2,1,Temp);
	}
	fclose(CStream);
	fclose(Temp);
	system("del Temp1.cip");
	return 0;
}

int FileDecryption(char *PlainFile,char *CipherFile,KeyType Key)
{//文件解密
    int i,n=0;
	BlockType Plain,Cipher,y=0;
	unsigned char w=0,v=0,Buf1,Buf2;
	FILE *PStream,*CStream,*Temp;
    CStream=fopen(CipherFile,"rb");
	if(!CStream){cout<<"文件 "<<CipherFile<<" 不存在!!!"<<endl;return 1;}
	Temp=fopen("Temp2.cip","wb");
	fread(&Cipher,2,1,CStream);
	while(!feof(CStream))
	{
        BlockDecryption(Plain,Cipher,Key);
		Plain^=y;
		y=Cipher;
		fwrite(&Plain,2,1,Temp);
        fread(&Cipher,2,1,CStream);
	}
	fclose(CStream);
	fclose(Temp);

        ///////*****    Temp	*****/////// 

    Temp=fopen("Temp2.cip","rb");
	if(!Temp){cout<<"Error!!!"<<endl;return 1;}
	fread(&Cipher,2,1,Temp);
	v=Cipher;
	Cipher>>=8;
	w=Cipher;
	for(i=0;i<w;i++)
	{
		fread(&PlainFile[i],1,1,Temp);
	}
    PlainFile[i]=0;
	PStream=fopen(PlainFile,"wb");
	fread(&Buf1,1,1,Temp);
    Buf2=Buf1;
	fread(&Buf1,1,1,Temp);
	while(!feof(Temp))
	{
		fwrite(&Buf2,1,1,PStream);
        Buf2=Buf1;
		fread(&Buf1,1,1,Temp);
	}
	if(!v)fwrite(&Buf2,1,1,PStream);
	fclose(Temp);
	fclose(PStream);
	system("del Temp2.cip");
	return 0;
}

unsigned long EncryptionTime(unsigned long Times)
{//加密函数的运行速度
	unsigned long toTime=0,t;
	BlockType text,Ctext;
	KeyType key={0};
	unsigned long i,j,k;
	srand(GetTickCount());
	for(j=0;j<Times;j++)
	{
		for(i=5;i>0;i--)key[i-1]=rand()%0x10000;
		t=GetTickCount();
		for(text=k=0x0;k<0x10000;text++,k++)BlockEncryption(text,Ctext,key);
		t=GetTickCount()-t;
		toTime+=t;		
	}
    return toTime/Times;
}

BlockType LinearCryptanalysis(unsigned long T,BlockType Text[][2])
{//线性密码分析
	unsigned short L,TTemp,Temp,z;
	unsigned long j,Count[256],max=0;
	for(TTemp=0;TTemp<0x100;TTemp++)Count[TTemp]=0;
	for(j=0;j<T;j++)
	{
		for(TTemp=0;TTemp<0x100;TTemp++)
		{
            Temp=(Text[j][1]&0x0f00);
			Temp>>=4;
			Temp|=(Text[j][1]&15);
            Temp^=TTemp;
			Temp=SBox_Decode(Temp);
			z=(Text[j][0]>>11);
			z^=(Text[j][0]>>9);
			z^=(Text[j][0]>>8);
			z^=(Temp>>6);
			z^=(Temp>>4);
			z^=(Temp>>2);
			z^=Temp;
			z&=1;
			if(!z)Count[TTemp]++;
		}	
	}
	for(TTemp=0;TTemp<0x100;TTemp++)
	{
        Count[TTemp]=(T/2>Count[TTemp]?T/2-Count[TTemp]:Count[TTemp]-T/2);
		if(Count[TTemp]>max){max=Count[TTemp];L=TTemp;}
	}
	z=L<<4;
	L|=z;
	L&=0x0f0f;
	return L;
}

BlockType DiffCryptanalysis(unsigned long T,BlockType Text[][4])
{//差分密码分析
    unsigned short L,TTemp,Temp1,Temp2,z;
	unsigned long j,Count[256],max=0;
	for(TTemp=0;TTemp<0x100;TTemp++)Count[TTemp]=0;
	for(j=0;j<T;j++)
	{
		for(TTemp=0;TTemp<0x100;TTemp++)
		{
            Temp1=(Text[j][1]&0x0f00);
			Temp1>>=4;
			Temp1|=(Text[j][1]&15);
            Temp1^=TTemp;
			Temp1=SBox_Decode(Temp1);
            Temp2=(Text[j][3]&0x0f00);
			Temp2>>=4;
			Temp2|=(Text[j][3]&15);
            Temp2^=TTemp;
			Temp2=SBox_Decode(Temp2);
            z=Temp1^Temp2;
			if(!((z^0x0066)&0x00ff))Count[TTemp]++;
		}
	}
	for(TTemp=0;TTemp<0x100;TTemp++)
	{
		if(Count[TTemp]<=max)continue;
		max=Count[TTemp];
		L=TTemp;
	}
    z=L<<4;
	L|=z;
	L&=0x0f0f;
	return L;
}

void TestBlockEncrypt()
{//测试分组加密与解密函数
	BlockType text,cipher,plain;
	cout<<endl<<endl<<"---------------测试分组加密与解密函数------------------"<<endl;
	Key_Engine();
	cout<<"输入一个0--65535的整数：";
	cin>>text;
	cout<<text<<"的二进制表示为：";OutPut_Bin(text);
    BlockEncryption(text,cipher,Key);
	cout<<endl<<"加密后的值为："<<cipher<<endl;
    cout<<cipher<<"的二进制表示为：";OutPut_Bin(cipher);
	cout<<endl<<"对"<<cipher<<"进行解密："<<endl;
    BlockDecryption(plain,cipher,Key);
	cout<<"解密后的值为："<<plain<<endl;
    cout<<plain<<"的二进制表示为：";OutPut_Bin(plain);
	if(!(text^plain))cout<<endl<<"成功！"<<endl;
	else cout<<endl<<"失败！"<<endl;
}

void TestFileEncrypt()
{//测试文件加密与解密函数
	int Error;
	char temp[128],cipher[128],plain[128];
	cout<<endl<<endl<<"---------------测试文件加密与解密函数------------------"<<endl;
	Key_Engine();
	cout<<"输入文件名：";
	cin>>temp;
    Error=FileEncryption(temp,cipher,Key);
	if(Error){cout<<"加密文件失败！";return;}
	cout<<"加密后的文件名为："<<cipher<<"，请手动查看。"<<endl;
	getchar();cout<<"按回车键继续......";while(getchar()!='\n'); 
	cout<<"对文件"<<cipher<<"进行解密："<<endl;
	Error=FileDecryption(plain,cipher,Key);
	if(Error){cout<<"解密文件失败！";return;}
    cout<<"已解密，文件名为"<<plain<<"，请手动查看"<<endl;
}

void TestEncryptionTime()
{//测试运行速度函数
	unsigned long AveTime=0,Times=1000;
	cout<<endl<<endl<<"---------------测试平均加密时间------------------"<<endl;
	cout<<"此过程大约需要"<<Times*150/(1000*60)<<"分钟，请耐心等待......"<<endl;
    AveTime=EncryptionTime(Times);
	cout<<"共对0--65535的值加密"<<Times<<"次，平均加密时间为 "<<AveTime<<"毫秒。"<<endl;
}

int TestLinearCryptanalysis(unsigned long T)
{//测试线性密码分析函数
	unsigned long i;
	Key_Random();
	//Key_Engine();
	BlockType (*textp)[2],text[20000][2];
	textp=text;
	srand(GetTickCount());
	for(i=0;i<T;i++)
	{
        text[i][0]=rand()%0xf000+0x1000;
		BlockEncryption(text[i][0],text[i][1],Key);
	}
    if(!((Key[4]^LinearCryptanalysis(T,textp))&0x0f0f))return 1;
	else return 0;
}

int TestDiffCryptanalysis(unsigned long T)
{//测试差分密码分析函数
	unsigned long i;
	Key_Random();
	//Key_Engine();
	BlockType (*textp)[4],text[1000][4];
	textp=text;
	srand(GetTickCount());
	for(i=0;i<T;i++)
	{
        text[i][0]=rand()%0x10000;
		BlockEncryption(text[i][0],text[i][1],Key);
		text[i][2]=text[i][0]^0x0b00;
		BlockEncryption(text[i][2],text[i][3],Key);
	}
    if(!((Key[4]^DiffCryptanalysis(T,textp))&0x0f0f))return 1;
	else return 0;
}

void TestLinearCryptanalysis()
{//测试线性密码分析函数
	unsigned long i,T=8000;
	BlockType Temp;
	cout<<endl<<endl<<"---------------测试线性密码分析------------------"<<endl;
	Key_Random();
    //Key_Engine();
	BlockType (*textp)[2],text[20000][2];
	textp=text;
	srand(GetTickCount());
	for(i=0;i<T;i++)
	{
        text[i][0]=rand()%0x10000;
		BlockEncryption(text[i][0],text[i][1],Key);
	}
	Temp=LinearCryptanalysis(T,textp);
	cout<<"所分析的密钥为  ";OutPut_Bin(Key[4]&0x0f0f);
	cout<<endl<<"分析得出的密钥为";OutPut_Bin(Temp);cout<<endl;
    if(!((Key[4]^Temp)&0x0f0f))cout<<"成功！"<<endl;
	else cout<<"失败！可能与随机明密文对的产生有关"<<endl;
}

void TestDiffCryptanalysis()
{//测试差分密码分析函数
	unsigned long i,T=100;
	BlockType Temp;
    cout<<endl<<endl<<"---------------测试差分密码分析------------------"<<endl;
	Key_Random();
	//Key_Engine();
	BlockType (*textp)[4],text[1000][4];
	textp=text;
	srand(GetTickCount());
	for(i=0;i<T;i++)
	{
        text[i][0]=rand()%0xf000+0x1000;
		BlockEncryption(text[i][0],text[i][1],Key);
		text[i][2]=text[i][0]^0x0b00;
		BlockEncryption(text[i][2],text[i][3],Key);
	}
	Temp=DiffCryptanalysis(T,textp);
	cout<<"所分析的密钥为  ";OutPut_Bin(Key[4]&0x0f0f);
	cout<<endl<<"分析得出的密钥为";OutPut_Bin(Temp);cout<<endl;
    if(!((Key[4]^Temp)&0x0f0f))cout<<"成功！"<<endl;
	else cout<<"失败！可能与随机明密文对的产生有关"<<endl;
}

void TestLinearSucceedTimes()
{//测试线性分析函数成功时明密文对数
    unsigned int n;
    cout<<endl<<endl<<"---------------测试线性分析函数成功时明密文对数-----------------"<<endl;
	for(n=6000;n<=20000;n+=200)
			if(TestLinearCryptanalysis(n))cout<<n<<"成功"<<endl;
			else cout<<n<<"失败"<<endl;
}

void TestDiffSucceedTimes()
{//测试差分分析函数成功时明密文对数
    unsigned int n;
    cout<<endl<<endl<<"---------------测试差分分析函数成功时明密文对数-----------------"<<endl;
	for(n=50;n<=300;n+=10)
			if(TestDiffCryptanalysis(n))cout<<n<<"成功"<<endl;
			else cout<<n<<"失败"<<endl;
}

void Wait()
{    
	getchar();
	cout<<"按回车键继续......";while(getchar()!='\n'); 
}

int main()
{
    TestBlockEncrypt();//分组加密解密
    Wait();
    TestFileEncrypt();//文件加密与解密
    Wait();
    TestEncryptionTime();//平均加密时间
    Wait();
    TestLinearCryptanalysis();//线性攻击
    Wait();
    TestDiffCryptanalysis();//差分攻击
    Wait();
    TestLinearSucceedTimes();//线性攻击成功所需的明密文对
    Wait();
    TestDiffSucceedTimes();//差分攻击成功所需的明密文对
    Wait();
    return 0;
} 