DragonTiger_RoomStockCFG = {}

function DragonTiger_RoomStockCFG:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_RoomStockCFG:init()
	self.datas = {}
	self.datas[1] = {RoomID = 1, Deduct_1 = 0.0, Deduct_2 = 0.0015, DefaultStock = 400000, Stock = {-100000,0,100000,200000,300000,400000,500000}, ChangeBuff = {1,2,3,4,5,6}, IsOpen = 1, RobotMinCount = 10, RobotMaxCount = 20, RobotMinVip = 0, RobotMaxVip = 4, RobotMinLifeTime = 900, RobotMaxLifeTime = 3600, RobotBankerGold = 5000000, RobotSystemMaxBet = 80000, RobotSystemMinBet = 30000, RobotPlayerMinBet = 5, RobotPlayerMaxBet = 10, RobotRobotMinBet = 5, RobotRobotMaxBet = 15}
	self.datas[2] = {RoomID = 2, Deduct_1 = 0.0, Deduct_2 = 0.0015, DefaultStock = 400000, Stock = {-100000,0,100000,200000,300000,400000,500000}, ChangeBuff = {1,2,3,4,5,6}, IsOpen = 1, RobotMinCount = 10, RobotMaxCount = 20, RobotMinVip = 0, RobotMaxVip = 4, RobotMinLifeTime = 900, RobotMaxLifeTime = 3600, RobotBankerGold = 5000000, RobotSystemMaxBet = 80000, RobotSystemMinBet = 30000, RobotPlayerMinBet = 5, RobotPlayerMaxBet = 10, RobotRobotMinBet = 5, RobotRobotMaxBet = 15}
	self.datas[3] = {RoomID = 3, Deduct_1 = 0.0, Deduct_2 = 0.0015, DefaultStock = 400000, Stock = {-100000,0,100000,200000,300000,400000,500000}, ChangeBuff = {1,2,3,4,5,6}, IsOpen = 1, RobotMinCount = 10, RobotMaxCount = 20, RobotMinVip = 0, RobotMaxVip = 4, RobotMinLifeTime = 900, RobotMaxLifeTime = 3600, RobotBankerGold = 5000000, RobotSystemMaxBet = 80000, RobotSystemMinBet = 30000, RobotPlayerMinBet = 5, RobotPlayerMaxBet = 10, RobotRobotMinBet = 5, RobotRobotMaxBet = 15}
	self.datas[4] = {RoomID = 4, Deduct_1 = 0.0, Deduct_2 = 0.0015, DefaultStock = 400000, Stock = {-100000,0,100000,200000,300000,400000,500000}, ChangeBuff = {1,2,3,4,5,6}, IsOpen = 1, RobotMinCount = 10, RobotMaxCount = 20, RobotMinVip = 0, RobotMaxVip = 4, RobotMinLifeTime = 900, RobotMaxLifeTime = 3600, RobotBankerGold = 5000000, RobotSystemMaxBet = 80000, RobotSystemMinBet = 30000, RobotPlayerMinBet = 5, RobotPlayerMaxBet = 10, RobotRobotMinBet = 5, RobotRobotMaxBet = 15}
	self.datas[5] = {RoomID = 5, Deduct_1 = 0.0, Deduct_2 = 0.0015, DefaultStock = 400000, Stock = {-100000,0,100000,200000,300000,400000,500000}, ChangeBuff = {1,2,3,4,5,6}, IsOpen = 1, RobotMinCount = 10, RobotMaxCount = 20, RobotMinVip = 0, RobotMaxVip = 4, RobotMinLifeTime = 900, RobotMaxLifeTime = 3600, RobotBankerGold = 5000000, RobotSystemMaxBet = 80000, RobotSystemMinBet = 30000, RobotPlayerMinBet = 5, RobotPlayerMaxBet = 10, RobotRobotMinBet = 5, RobotRobotMaxBet = 15}
end

DragonTiger_RoomStockCFG:init()
