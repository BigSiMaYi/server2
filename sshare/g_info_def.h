/*
  add by Hunter 2017/07/21;
  存放一些功能的定义各个服务之间;
*/
#ifndef  XX_G_INFO_DEF___XXXX
#define  XX_G_INFO_DEF___XXXX

//@add by Hunter 2017/06/30;
//@添加用户商品信息;
struct tagBagItem {
	int32_t itemId;
	int32_t itemCnt;
	int32_t endtime;
};

//@游戏操控信息;
struct tagOpItem {
	int32_t gameID;
	int32_t flagsA;
	double winsPercentA;

	int32_t flagsB;
	double winsPercentB;

	int32_t flagsC;
	double winsPercentC;

	//int64_t flagsY;
	bool	flagsS;

	double  opCoefficient;
	int64_t flagsMoneyLimte;
	int64_t curMoneyGet;

	//@标记是否已后台数据做标签;
	int32_t	gmOpFlags;
};

struct tagStaticOp {
	int64_t   SendGiftCoinCount;
	int64_t   RecvGiftCoinTotal;
	int64_t   RechargeCoinTotal;
	int64_t   withDrawCoinTotal;	//@提现总数;
	int32_t	  flagsY;				//@Y标签;
	int64_t	  flagsX;				//@X标签;
	int64_t   safeBag;				//@银行金币;
	std::map<int, std::shared_ptr<tagOpItem>> opItems;
};
#endif
