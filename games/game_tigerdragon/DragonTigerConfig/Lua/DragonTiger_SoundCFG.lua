DragonTiger_SoundCFG = {}

function DragonTiger_SoundCFG:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_SoundCFG:init()
	self.datas = {}
	self.datas[1] = {SoundID = 1, SoundName = "背景音乐", SoundPath = "Sound/brnn_bg.mp3", SoundTime = 32}
	self.datas[2] = {SoundID = 2, SoundName = "没牛", SoundPath = "Sound/sound_brnn_type_00.mp3", SoundTime = 2}
	self.datas[3] = {SoundID = 3, SoundName = "牛一", SoundPath = "Sound/sound_brnn_type_01.mp3", SoundTime = 2}
	self.datas[4] = {SoundID = 4, SoundName = "牛二", SoundPath = "Sound/sound_brnn_type_02.mp3", SoundTime = 2}
	self.datas[5] = {SoundID = 5, SoundName = "牛三", SoundPath = "Sound/sound_brnn_type_03.mp3", SoundTime = 2}
	self.datas[6] = {SoundID = 6, SoundName = "牛四", SoundPath = "Sound/sound_brnn_type_04.mp3", SoundTime = 2}
	self.datas[7] = {SoundID = 7, SoundName = "牛五", SoundPath = "Sound/sound_brnn_type_05.mp3", SoundTime = 2}
	self.datas[8] = {SoundID = 8, SoundName = "牛六", SoundPath = "Sound/sound_brnn_type_06.mp3", SoundTime = 2}
	self.datas[9] = {SoundID = 9, SoundName = "牛七", SoundPath = "Sound/sound_brnn_type_07.mp3", SoundTime = 2}
	self.datas[10] = {SoundID = 10, SoundName = "牛八", SoundPath = "Sound/sound_brnn_type_08.mp3", SoundTime = 2}
	self.datas[11] = {SoundID = 11, SoundName = "牛九", SoundPath = "Sound/sound_brnn_type_09.mp3", SoundTime = 2}
	self.datas[12] = {SoundID = 12, SoundName = "牛牛", SoundPath = "Sound/sound_brnn_type_10.mp3", SoundTime = 2}
	self.datas[13] = {SoundID = 13, SoundName = "银牛", SoundPath = "Sound/sound_brnn_type_11.mp3", SoundTime = 1}
	self.datas[14] = {SoundID = 14, SoundName = "四炸", SoundPath = "Sound/sound_brnn_type_12.mp3", SoundTime = 2}
	self.datas[15] = {SoundID = 15, SoundName = "五花牛", SoundPath = "Sound/sound_brnn_type_13.mp3", SoundTime = 2}
	self.datas[16] = {SoundID = 16, SoundName = "五小牛", SoundPath = "Sound/sound_brnn_type_14.mp3", SoundTime = 2}
	self.datas[17] = {SoundID = 17, SoundName = "倒计时音效", SoundPath = "Sound/count_down.mp3", SoundTime = 1}
	self.datas[18] = {SoundID = 18, SoundName = "倒计时3", SoundPath = "Sound/countdown_3.mp3", SoundTime = 1}
	self.datas[19] = {SoundID = 19, SoundName = "倒计时2", SoundPath = "Sound/countdown_2.mp3", SoundTime = 1}
	self.datas[20] = {SoundID = 20, SoundName = "倒计时1", SoundPath = "Sound/countdown_1.mp3", SoundTime = 1}
	self.datas[21] = {SoundID = 21, SoundName = "倒计时0开牌", SoundPath = "Sound/countdown_open.mp3", SoundTime = 1}
	self.datas[22] = {SoundID = 22, SoundName = "发牌音效", SoundPath = "Sound/sound-fapai.mp3", SoundTime = 1}
	self.datas[23] = {SoundID = 23, SoundName = "翻牌音效", SoundPath = "Sound/sound-fanpai.mp3", SoundTime = 1}
	self.datas[24] = {SoundID = 24, SoundName = "开始下注", SoundPath = "Sound/sound-start-wager.mp3", SoundTime = 2}
	self.datas[25] = {SoundID = 25, SoundName = "庄家轮换", SoundPath = "Sound/zhuang-2.mp3", SoundTime = 2}
	self.datas[26] = {SoundID = 26, SoundName = "筹码飞入音效", SoundPath = "Sound/sound-jetton.mp3", SoundTime = 1}
	self.datas[27] = {SoundID = 27, SoundName = "结算获胜音乐", SoundPath = "Sound/sound-result-win.mp3", SoundTime = 5}
	self.datas[28] = {SoundID = 28, SoundName = "赢钱音效1", SoundPath = "Sound/girl_win_0.mp3", SoundTime = 2}
	self.datas[29] = {SoundID = 29, SoundName = "赢钱音效2", SoundPath = "Sound/girl_win_1.mp3", SoundTime = 3}
	self.datas[30] = {SoundID = 30, SoundName = "赢钱音效3", SoundPath = "Sound/girl_win_2.mp3", SoundTime = 3}
	self.datas[31] = {SoundID = 31, SoundName = "结算失败音乐", SoundPath = "Sound/lose-sound.mp3", SoundTime = 2}
	self.datas[32] = {SoundID = 32, SoundName = "输钱音效1", SoundPath = "Sound/girl_lost_1.mp3", SoundTime = 3}
	self.datas[33] = {SoundID = 33, SoundName = "输钱音效2", SoundPath = "Sound/girl_lost_2.mp3", SoundTime = 3}
	self.datas[34] = {SoundID = 34, SoundName = "输钱音效3", SoundPath = "Sound/girl_lost_3.mp3", SoundTime = 3}
	self.datas[35] = {SoundID = 35, SoundName = "结算获胜音乐", SoundPath = "Sound/sound-result-win.mp3", SoundTime = 5}
	self.datas[36] = {SoundID = 36, SoundName = "赢钱音效1", SoundPath = "Sound/girl_win_0.mp3", SoundTime = 2}
	self.datas[37] = {SoundID = 37, SoundName = "赢钱音效2", SoundPath = "Sound/girl_win_1.mp3", SoundTime = 3}
	self.datas[38] = {SoundID = 38, SoundName = "赢钱音效3", SoundPath = "Sound/girl_win_2.mp3", SoundTime = 3}
	self.datas[39] = {SoundID = 39, SoundName = "结算失败音乐", SoundPath = "Sound/lose-sound.mp3", SoundTime = 2}
	self.datas[40] = {SoundID = 40, SoundName = "输钱音效1", SoundPath = "Sound/girl_lost_1.mp3", SoundTime = 3}
	self.datas[41] = {SoundID = 41, SoundName = "输钱音效2", SoundPath = "Sound/girl_lost_2.mp3", SoundTime = 3}
	self.datas[42] = {SoundID = 42, SoundName = "输钱音效3", SoundPath = "Sound/girl_lost_3.mp3", SoundTime = 3}
end

DragonTiger_SoundCFG:init()
