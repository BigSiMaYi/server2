DragonTiger_WeightCFG = {}

function DragonTiger_WeightCFG:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_WeightCFG:init()
	self.datas = {}
	self.datas[1] = {Index = 1, Water = 0.7, KillScoreRate = 100, TriggerControl = 1}
	self.datas[2] = {Index = 2, Water = 0.8, KillScoreRate = 82, TriggerControl = 1}
	self.datas[3] = {Index = 3, Water = 0.9, KillScoreRate = 83, TriggerControl = 1}
	self.datas[4] = {Index = 4, Water = 1.1, KillScoreRate = 84, TriggerControl = 1}
	self.datas[5] = {Index = 5, Water = 1.2, KillScoreRate = 85, TriggerControl = 0}
	self.datas[6] = {Index = 6, Water = 1.3, KillScoreRate = 100, TriggerControl = 1}
end

DragonTiger_WeightCFG:init()
