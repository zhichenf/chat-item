#ifndef PUBLIC_H_
#define PUBLIC_H_

/*
*server 和 client的公共文件
*/

enum EnMsgType {
    kLoginMsg = 1,      //登录消息
    kLoginMsgAck,       //登录回应
    kRegMsg,            //注册消息
    kRegMsgAck,         //注册回应
    kOneChatMsg,        //1对1聊天消息
    kAddFriendMsg,      //添加好友消息  
    kAddFriendMsgAck,   //添加好友消息回应

    kNotifyFriend,      //通知好友用户上线
    kNotifyFriendExit,	//通知好友用户下线

    kCreateGroupMsg,    //创建群组消息
    kCreateGroupMsgAck,	//创建群组消息回应
    kAddGroupMsg,       //加入群组消息
    kAddGroupMsgAck,    //加入群组消息回应
    kGroupChatMsg,      //群聊天
};

#endif