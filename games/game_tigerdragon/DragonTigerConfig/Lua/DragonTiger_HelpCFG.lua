DragonTiger_HelpCFG = {}

function DragonTiger_HelpCFG:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_HelpCFG:init()
	self.datas = {}
	self.datas[1] = {HelpID = 1, CardsName = "没牛", CardsInfo = "3张相加不为10的倍数 倍率1", CardsTypeStr = "@A", Pokers = {37,40,42,2,52}}
	self.datas[2] = {HelpID = 2, CardsName = "牛二", CardsInfo = "3张相加为10的倍数，另外两张相加各位数为2 倍率1", CardsTypeStr = "02", Pokers = {37,49,52,2,49}}
	self.datas[3] = {HelpID = 3, CardsName = "牛牛", CardsInfo = "3张相加为10的倍数，另外两张相加为10的倍数 倍率5", CardsTypeStr = "00", Pokers = {37,40,49,9,52}}
	self.datas[4] = {HelpID = 4, CardsName = "四炸", CardsInfo = "五张牌中有4张完全一样 倍率7", CardsTypeStr = "<=", Pokers = {31,18,44,52,44}}
	self.datas[5] = {HelpID = 5, CardsName = "五花牛", CardsInfo = "五张牌为花牌(J、Q、K) 倍率8", CardsTypeStr = ">B0", Pokers = {37,50,51,13,24}}
	self.datas[6] = {HelpID = 6, CardsName = "五小牛", CardsInfo = "五张牌均小于5，且相加之和小于10 倍率9", CardsTypeStr = ">?0", Pokers = {28,15,40,3,41}}
end

DragonTiger_HelpCFG:init()
