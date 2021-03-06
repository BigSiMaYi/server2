DragonTiger_RoomCFG = {}

function DragonTiger_RoomCFG:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_RoomCFG:init()
	self.datas = {}
	self.datas[1] = {RoomID = 1, RoomName = "初级场", RoomImage = "UI/Image/RoomIcon1.png", GoldCondition = 1000, VipCondition = 0, TicketCondition = 0, MaxRate = 500000, ChipList = {100,500,10000,100000,500000}, UnlockChipList = {0,5000,100000,1000000,5000000}, ChipImages = {"UI/Image/Chips1.png","UI/Image/Chips2.png","UI/Image/Chips3.png","UI/Image/Chips4.png","UI/Image/Chips5.png"}, IsOpen = true, BankerGold = 10, SnatchGold = 500000, ForceLeaveGold = 3000000}
	self.datas[2] = {RoomID = 2, RoomName = "中级场", RoomImage = "UI/Image/RoomIcon2.png", GoldCondition = 100000, VipCondition = 0, TicketCondition = 0, MaxRate = 10000000, ChipList = {1000,10000,100000,1000000,5000000}, UnlockChipList = {10000,100000,1000000,10000000,50000000}, ChipImages = {"UI/Image/Chips1.png","UI/Image/Chips2.png","UI/Image/Chips3.png","UI/Image/Chips4.png","UI/Image/Chips5.png"}, IsOpen = true, BankerGold = 10, SnatchGold = 1000000, ForceLeaveGold = 10000000}
end

DragonTiger_RoomCFG:init()
