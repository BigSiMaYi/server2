DragonTiger_RobotCFG = {}

function DragonTiger_RobotCFG:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_RobotCFG:init()
	self.datas = {}
	self.datas["RobotGold0"] = {Key = "RobotGold0", Value = 300000}
	self.datas["RobotGold1"] = {Key = "RobotGold1", Value = 500000}
	self.datas["RobotGold2"] = {Key = "RobotGold2", Value = 1000000}
	self.datas["RobotGold3"] = {Key = "RobotGold3", Value = 3000000}
	self.datas["RobotGold4"] = {Key = "RobotGold4", Value = 10000000}
	self.datas["RobotBankerCD"] = {Key = "RobotBankerCD", Value = 900}
	self.datas["RobotBankerMinCount"] = {Key = "RobotBankerMinCount", Value = 6}
	self.datas["RobotBankerMaxCount"] = {Key = "RobotBankerMaxCount", Value = 20}
end

DragonTiger_RobotCFG:init()
