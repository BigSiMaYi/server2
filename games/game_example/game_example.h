// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� GAME_EXAMPLE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// GAME_FISHLORD_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef GAME_EXAMPLE_EXPORTS
#define GAME_EXAMPLE_API __declspec(dllexport)
#else
#define GAME_EXAMPLE_API __declspec(dllimport)
#endif

extern "C" {

	GAME_EXAMPLE_API void* get_game_engine();

	GAME_EXAMPLE_API void* get_packet_mgr();
}
