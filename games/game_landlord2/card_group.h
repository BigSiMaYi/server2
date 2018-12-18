#pragma once
enum Type
{
    Unkown,        //δ֪;
    Single,            //����;
    Double,          //����;
    Three,             //����;
    SingleSeq,      //��˳;
    DoubleSeq,    //˫˳;
    ThreeSeq,       //��˳;
    ThreePlus,      //����һ��һ�Ż�һ�ԣ�;
    Airplane,         //�ɻ�;
    FourSeq,         //�Ĵ��������Ż����ԣ�;
    Bomb,            //ը������ը;
};

//���ͽṹ;
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
    void AddNumber(int num);        //���0-53��ʾ����Ԫ��;
    void DeleteNumber(int num);     //ȥ��һ����;
    void Clear(void);                          //���ô˽ṹ;

    static int Translate(int num);       //��0-53ת����3-17Ȩֵ������A��14����2��15����С����16����������17��;

public:
    std::map<int, int> group_;      //3-17Ȩֵ����;
    std::set<int> cards_;                //0-53��ɵļ��ϣ���Ҫ���ڷ��㻭����ʾ;
    Type type_;                              //�������ͣ����ơ����ӵȵȣ�;
    int value_;                                //Ȩֵ;
    int count_;                               //�˽ṹԪ����������������;
};