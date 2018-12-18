DragonTiger_Cardodds = {}

function DragonTiger_Cardodds:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_Cardodds:init()
	self.datas = {}
	self.datas["win_queal"] = {Key = "win_queal", Value = 8.0}
	self.datas["dragon"] = {Key = "dragon", Value = 0.5}
	self.datas["tiger"] = {Key = "tiger", Value = 0.5}
	self.datas["win_dragon"] = {Key = "win_dragon", Value = 1.0}
	self.datas["win_tiger"] = {Key = "win_tiger", Value = 1.0}
	self.datas["lose_dragon"] = {Key = "lose_dragon", Value = 1.0}
	self.datas["lose_tiger"] = {Key = "lose_tiger", Value = 1.0}
end

DragonTiger_Cardodds:init()
