#pragma once
enum Type
{
    Unkown,        //未知;
    Single,            //单张;
    Double,          //对子;
    Three,             //三条;
    SingleSeq,      //单顺;
    DoubleSeq,    //双顺;
    ThreeSeq,       //三顺;
    ThreePlus,      //三带一（一张或一对）;
    Airplane,         //飞机;
    FourSeq,         //四带二（两张或两对）;
    Bomb,            //炸弹、王炸;
};

//牌型结构;
class CardGroup
{
    friend class card_analyse;
    friend class Game;
    friend class Scene;

public:
    CardGroup();
    CardGroup(Type t, int v);
    CardGroup& operator=(CardGroup &cg);

public:
    void AddNumber(int num);        //添加0-53表示的牌元素;
    void DeleteNumber(int num);     //去掉一张牌;
    void Clear(void);                          //重置此结构;

    static int Translate(int num);       //把0-53转换成3-17权值，其中A（14）、2（15）、小王（16）、大王（17）;

public:
    std::map<int, int> group_;      //3-17权值集合;
    std::set<int> cards_;                //0-53组成的集合，主要用于方便画面显示;
    Type type_;                              //牌型类型（单牌、对子等等）;
    int value_;                                //权值;
    int count_;                               //此结构元素数量（牌数量）;
};