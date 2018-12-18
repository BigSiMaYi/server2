DragonTiger_MultiLanguageCFG = {}

function DragonTiger_MultiLanguageCFG:getData(key)
	if self.datas == nil then
		return nil
	end
	return self.datas[key]
end

function DragonTiger_MultiLanguageCFG:init()
	self.datas = {}
	self.datas["Room_NeedVipLevel"] = {ID = "Room_NeedVipLevel", Name = "需要VIP%d解锁"}
	self.datas["Room_RateLimit"] = {ID = "Room_RateLimit", Name = "最大下注：%s"}
	self.datas["Main_Prepare"] = {ID = "Main_Prepare", Name = "正在准备开始下一局……"}
	self.datas["Main_Prepare_Banker"] = {ID = "Main_Prepare_Banker", Name = "正在准备开始下一局……，本局你是庄家。"}
	self.datas["Main_Prepare_Leave"] = {ID = "Main_Prepare_Leave", Name = "正在准备开始下一局……，本局你已经下庄。"}
	self.datas["Main_StartBet"] = {ID = "Main_StartBet", Name = "开始下注。"}
	self.datas["Main_Win"] = {ID = "Main_Win", Name = "胜"}
	self.datas["Main_Lose"] = {ID = "Main_Lose", Name = "败"}
	self.datas["Apply_Banker"] = {ID = "Apply_Banker", Name = "申请上庄需要%s金币！\n~充值能获得充足的金币哦！"}
	self.datas["Tips_NotEnoughGold"] = {ID = "Tips_NotEnoughGold", Name = "金币不足，不能下注。"}
	self.datas["Tips_ApplySuccess"] = {ID = "Tips_ApplySuccess", Name = "申请上庄成功。"}
	self.datas["Tips_ApplyFail_Gold"] = {ID = "Tips_ApplyFail_Gold", Name = "申请上庄失败，金币不足。"}
	self.datas["Tips_ApplyFail_IsBanker"] = {ID = "Tips_ApplyFail_IsBanker", Name = "申请上庄失败，你已经是庄家。"}
	self.datas["Tips_ApplyFail_InBankerList"] = {ID = "Tips_ApplyFail_InBankerList", Name = "申请上庄失败，你已经在庄家列表中。"}
	self.datas["Tips_ApplyFail_Full"] = {ID = "Tips_ApplyFail_Full", Name = "申请上庄失败，申请上庄人数已满。"}
	self.datas["Tips_LeaveSuccess"] = {ID = "Tips_LeaveSuccess", Name = "申请下庄成功，本局结束后将下庄。"}
	self.datas["Tips_LeaveFail_IsNotBanker"] = {ID = "Tips_LeaveFail_IsNotBanker", Name = "申请下庄失败，你不是庄家。"}
	self.datas["Tips_LeaveFail_NotEnoughTicket"] = {ID = "Tips_LeaveFail_NotEnoughTicket", Name = "申请下庄失败，你没有足够的钻石。"}
	self.datas["Tips_LeaveFail_Applied"] = {ID = "Tips_LeaveFail_Applied", Name = "你已申请下庄，不要重复。"}
	self.datas["Tips_ForceLeave"] = {ID = "Tips_ForceLeave", Name = "{fontname=Common2/FZY3JW.TTF,fontsize=30,color=0xFF93523D}上庄时间不够，是否强制下庄，强制下庄需要10钻石。"}
	self.datas["Tips_CantBet_IsBanker"] = {ID = "Tips_CantBet_IsBanker", Name = "你是庄家，不能下注。"}
	self.datas["Tips_LeaveRoom_IsBanker"] = {ID = "Tips_LeaveRoom_IsBanker", Name = "你是庄家，不能离开房间。"}
	self.datas["Tips_ErrorState"] = {ID = "Tips_ErrorState", Name = "下注失败，错误状态。"}
	self.datas["Tips_Bet_Banker_Is_Full"] = {ID = "Tips_Bet_Banker_Is_Full", Name = "下注失败，庄家下注金额已满。"}
	self.datas["Tips_Bet_Other_Is_Full"] = {ID = "Tips_Bet_Other_Is_Full", Name = "每轮下注金额不得超过现金的10%"}
	self.datas["Total_Bet_Info"] = {ID = "Total_Bet_Info", Name = "总下注："}
	self.datas["Total_Info"] = {ID = "Total_Info", Name = "总计"}
	self.datas["Snatch_Can"] = {ID = "Snatch_Can", Name = "可以竞价抢庄。"}
	self.datas["Snatch_Is_High"] = {ID = "Snatch_Is_High", Name = "你的竞价最高。"}
	self.datas["Snatch_Is_Not_High"] = {ID = "Snatch_Is_Not_High", Name = "你的竞价被超过。\n当前竞价是%d"}
	self.datas["Snatch_Is_Cur"] = {ID = "Snatch_Is_Cur", Name = "当前竞价是%d。"}
	self.datas["Snatch_Tips"] = {ID = "Snatch_Tips", Name = "{fontname=Common2/FZY3JW.TTF,fontsize=30,color=0xFF93523D}      是否花费%d金币竞价抢庄！            {newline,fontsize=24}    (如果无人超过您的出价，下一局即可上庄){newline}    tip:竞价失败不会扣除金币"}
	self.datas["Snatch_success"] = {ID = "Snatch_success", Name = "竞价抢庄成功。"}
	self.datas["Snatch_Protect"] = {ID = "Snatch_Protect", Name = "庄家在保护期，不能竞价抢庄。"}
	self.datas["Snatch_Is_Self"] = {ID = "Snatch_Is_Self", Name = "你的竞价已经是最高。"}
	self.datas["Snatch_Is_Low"] = {ID = "Snatch_Is_Low", Name = "你的竞价过低。"}
	self.datas["Snatch_Is_Banker"] = {ID = "Snatch_Is_Banker", Name = "你是庄家，不能竞价。"}
	self.datas["Snatch_Not_Enough_Ticket"] = {ID = "Snatch_Not_Enough_Ticket", Name = "钻石不够，不能竞价。"}
	self.datas["Balance_Not_Bet"] = {ID = "Balance_Not_Bet", Name = "未投注"}
	self.datas["System_Banker_Name"] = {ID = "System_Banker_Name", Name = "系统小庄"}
	self.datas["Tips_Pre_Balance"] = {ID = "Tips_Pre_Balance", Name = "正在结算，请稍候…"}
end

DragonTiger_MultiLanguageCFG:init()
