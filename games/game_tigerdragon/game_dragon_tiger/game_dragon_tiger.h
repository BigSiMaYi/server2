// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� GAME_FISHLORD_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// GAME_FISHLORD_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef GAME_DRAGON_TIGER_EXPORTS
#define GAME_DRAGON_TIGER_API __declspec(dllexport)
#else
#define GAME_DRAGON_TIGER_API __declspec(dllimport)
#endif

extern "C" {

	GAME_DRAGON_TIGER_API void* get_game_engine();

	GAME_DRAGON_TIGER_API void* get_packet_mgr();
}
