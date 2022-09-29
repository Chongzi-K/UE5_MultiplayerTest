# UE5_MultiplayerTest

1、制作用于Steam联机的UE插件

#参考项目：https://github.com/DruidMech/MultiplayerCourseBlasterGame

            https://www.udemy.com/course/unreal-engine-5-cpp-multiplayer-shooter/

2、将旧项目用可联网的方式重制

#参考教程:  
            https://www.bilibili.com/video/BV1R34y1Q7b6 ——————安宁Ken (RPC)
			
			https://docs.unrealengine.com/5.0/zh-CN/rpcs-in-unreal-engine/
			
            https://www.bilibili.com/video/BV1ED4y1D7Sf ——————[中文直播]第26期 | 虚幻引擎GamePlay框架理解与应用 | Epic 大钊 马骥
			
			https://www.bilibili.com/video/BV1ti4y1r7rS ——————[英文直播]和Tom Looman一起开发C++游戏玩法框架(官方字幕)
			
			https://zhuanlan.zhihu.com/p/23480071       ——————大钊：《InsideUE4》GamePlay架构（五）Controller
			
			https://zhuanlan.zhihu.com/p/361520144      ——————逆运动学(六): FABRIK （20220929）
			
------------------		
https://www.bilibili.com/video/BV1Wa411J7w3 ——————[英文直播]生存游戏《Derelicts》是如何诞生的(官方字幕)


------------------
20220926-1

1、添加Weapon与玩家的基本互动

------------------
20220925-2

1、添加玩家HUD

#TODO 未开启 Draw at Desire Size，需要后期按需调整 Wiget 大小以适应结果

------------------
20220925-1

1、完善基本人物控制与动画

------------------
20220924-6

1、新建项目 MultiplayerProject ，并从上个项目搬运插件到新项目

------------------
20220924-5

1、完善按钮功能：防止多次连按、功能执行失败再次恢复

2、完善 DestroySession 逻辑与委托绑定

------------------
20220924-4

1、将地图文件地址软编码

------------------
20220924-3

1、新增GameMode_Lobby，在地图Lobby中使用GameMode_Lobby，主要负责显示进出玩家的信息

------------------
20220924-2

1、完善在FindSession中调用JoinSession的逻辑与委托绑定

------------------
20220924-1

1、MultiplayerSessionsSubsystem 中声明所有要用到的 委托类型 + 定义委托变量 

①CreatSession   ——动态多播

②FindSession    ——多播

③JoinSession    ——多播

④DestroySession ——动态多播

⑤StartSession   ——动态多播

#动态需要回调函数为 UFNCTION()

2、Menu 中声明5个回调函数并绑定委托

#非动态委托的绑定要用 AddUObject 而不是 AddDynamic 

------------------
20220923-5

1、在 Menu 中使用 OnLevelRemovedFromWorld 调用 MenuTearDown ，实现在关卡切换时更新控制模式

------------------
20220923-4

1、新增 Menu 类，实现按钮触发 CreatSession 与 JoinSession 的功能

2、完善 MultiplayerSessionsSubsystem 中 CreateSession

------------------
20220923-3

1、在 MultiplayerSessionsSubsystem 中声明委托回调函数，并在cpp中定义

2、在 MultiplayerSessionsSubsystem 构造函数中绑定本类中的委托回调函数

------------------
20220923-2

1、新建 OnlineSubsystem