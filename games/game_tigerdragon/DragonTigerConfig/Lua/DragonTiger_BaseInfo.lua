DragonTiger_BaseInfo = {}

function DragonTiger_BaseInfo:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_BaseInfo:init()
	self.datas = {}
	self.datas["BetRate"] = {Key = "BetRate", Value = 10}
	self.datas["BankerBetRate"] = {Key = "BankerBetRate", Value = 5}
	self.datas["PreTime"] = {Key = "PreTime", Value = 3}
	self.datas["BetTime"] = {Key = "BetTime", Value = 15}
	self.datas["DealTime"] = {Key = "DealTime", Value = 23}
	self.datas["ResultTime"] = {Key = "ResultTime", Value = 6}
	self.datas["MinBankerCount"] = {Key = "MinBankerCount", Value = 1}
	self.datas["CostLeaveBanker"] = {Key = "CostLeaveBanker", Value = 5}
	self.datas["ForceLeaveCost"] = {Key = "ForceLeaveCost", Value = 0}
	self.datas["WinStarCount"] = {Key = "WinStarCount", Value = 0}
	self.datas["AwardGetRate"] = {Key = "AwardGetRate", Value = 100}
	self.datas["BroadcastGold"] = {Key = "BroadcastGold", Value = 2000000}
	self.datas["PlayerBankerCount"] = {Key = "PlayerBankerCount", Value = 10}
end

DragonTiger_BaseInfo:init()
