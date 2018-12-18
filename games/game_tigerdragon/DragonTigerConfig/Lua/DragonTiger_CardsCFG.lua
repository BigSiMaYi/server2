DragonTiger_CardsCFG = {}

function DragonTiger_CardsCFG:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_CardsCFG:init()
	self.datas = {}
	self.datas[0] = {CardsID = 0, CardsName = "没牛", CardsRate = 1, CardsTypeStr = "@A", SoundID = 2, CardsAnimation = ""}
	self.datas[1] = {CardsID = 1, CardsName = "牛1", CardsRate = 1, CardsTypeStr = "01", SoundID = 3, CardsAnimation = ""}
	self.datas[2] = {CardsID = 2, CardsName = "牛2", CardsRate = 1, CardsTypeStr = "02", SoundID = 4, CardsAnimation = ""}
	self.datas[3] = {CardsID = 3, CardsName = "牛3", CardsRate = 1, CardsTypeStr = "03", SoundID = 5, CardsAnimation = ""}
	self.datas[4] = {CardsID = 4, CardsName = "牛4", CardsRate = 1, CardsTypeStr = "04", SoundID = 6, CardsAnimation = ""}
	self.datas[5] = {CardsID = 5, CardsName = "牛5", CardsRate = 1, CardsTypeStr = "05", SoundID = 7, CardsAnimation = ""}
	self.datas[6] = {CardsID = 6, CardsName = "牛6", CardsRate = 1, CardsTypeStr = "06", SoundID = 8, CardsAnimation = ""}
	self.datas[7] = {CardsID = 7, CardsName = "牛7", CardsRate = 2, CardsTypeStr = "07", SoundID = 9, CardsAnimation = ""}
	self.datas[8] = {CardsID = 8, CardsName = "牛8", CardsRate = 3, CardsTypeStr = "08", SoundID = 10, CardsAnimation = ""}
	self.datas[9] = {CardsID = 9, CardsName = "牛9", CardsRate = 4, CardsTypeStr = "09", SoundID = 11, CardsAnimation = ""}
	self.datas[10] = {CardsID = 10, CardsName = "牛牛", CardsRate = 5, CardsTypeStr = "00", SoundID = 12, CardsAnimation = ""}
	self.datas[11] = {CardsID = 11, CardsName = "银牛", CardsRate = 6, CardsTypeStr = ";0", SoundID = 13, CardsAnimation = "NIUNIUyinniudonghua"}
	self.datas[12] = {CardsID = 12, CardsName = "四炸", CardsRate = 7, CardsTypeStr = "<=", SoundID = 14, CardsAnimation = "niuniubaozhatexiao"}
	self.datas[13] = {CardsID = 13, CardsName = "五花牛", CardsRate = 8, CardsTypeStr = ">B0", SoundID = 15, CardsAnimation = "NIUNIUjinniudonghua"}
	self.datas[14] = {CardsID = 14, CardsName = "五小牛", CardsRate = 9, CardsTypeStr = ">?0", SoundID = 16, CardsAnimation = "NIUNIUwuxiaoniudonghua"}
end

DragonTiger_CardsCFG:init()
