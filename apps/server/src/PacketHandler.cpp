#include "Pch.h"
#include "PacketHandler.h"
#include "DbWorker.h"

#include "ClientSession.h"
#include "network/Protocol.h"
#include "network/PacketDispatcher.h"

//  TODO
void PacketHandler::RegisterPacketHandleFunc(DbWorker* dbWorker)
{
    RegisterCS2RegisterHandleFunc(dbWorker);
    RegisterCS2LoginHandleFunc(dbWorker);
    RegisterCS2ResumeHandleFunc(dbWorker);
}

 void PacketHandler::RegisterCS2RegisterHandleFunc(DbWorker* dbWorker)
 {
    network::PacketDispatcher::Register<Protocol::C2S_Register>(Protocol::PacketId::C2S_Register,
        [dbWorker](std::shared_ptr<network::Session>& session, const Protocol::C2S_Register& registerPacket)
        {
            dbWorker->Enqueue([session, username = registerPacket.username, password = registerPacket.username](AuthService& auth){
                
                if(session == nullptr)
                    return;
                
                uint64 newUserId = 0;
                AuthFailReason authFailReason;

                bool registerSuccessed   = auth.RegisterUser(username, password, newUserId, authFailReason);

                if(registerSuccessed == false)
                {
                    Protocol::S2C_AuthFail authfailPacket;
                    authfailPacket.reason = static_cast<uint16>(authFailReason);
                    Protocol::SendPacket(session, Protocol::PacketId::S2C_AuthFail, authfailPacket);
                    return;
                }

                LoginResult loginResult;
                AuthFailReason authFailReason;
                bool loginSuccessed = auth.Login(username, password, loginResult, authFailReason);

                if(loginSuccessed == false)
                {
                    Protocol::S2C_AuthFail authfailPacket;
                    authfailPacket.reason = static_cast<uint16>(authFailReason);
                    Protocol::SendPacket(session, Protocol::PacketId::S2C_AuthFail, authfailPacket);
                    return;
                }

                {
                    auto clientSession = std::static_pointer_cast<ClientSession>(session);
                    clientSession->SetUserId(loginResult.userId);
                }

                Protocol::S2C_AuthOk autoOkPacket;
                autoOkPacket.userId = loginResult.userId;
                autoOkPacket.expiresAt = loginResult.expiresAt;
                autoOkPacket.token = loginResult.token;
                Protocol::SendPacket(session, Protocol::PacketId::S2C_AuthOk, autoOkPacket);

            });
        });
 }

void PacketHandler::RegisterCS2RegisterHandleFunc(DbWorker* dbWorker)
{
    network::PacketDispatcher::Register<Protocol::C2S_Login>(Protocol::PacketId::C2S_Login,
        [dbWorker](std::shared_ptr<network::Session>& session, const Protocol::C2S_Login& loginPacket)
        {
            dbWorker->Enqueue([session, username = loginPacket.username, password = loginPacket.username](AuthService& auth){
                
                if(session == nullptr)
                    return;
                
                LoginResult loginResult;
                AuthFailReason authFailReason;
                bool loginSuccessed = auth.Login(username, password, loginResult, authFailReason);

                if(loginSuccessed == false)
                {
                    Protocol::S2C_AuthFail authfailPacket;
                    authfailPacket.reason = static_cast<uint16>(authFailReason);
                    Protocol::SendPacket(session, Protocol::PacketId::S2C_AuthFail, authfailPacket);
                    return;
                }

                {
                    auto clientSession = std::static_pointer_cast<ClientSession>(session);
                    clientSession->SetUserId(loginResult.userId);
                }

                Protocol::S2C_AuthOk autoOkPacket;
                autoOkPacket.userId = loginResult.userId;
                autoOkPacket.expiresAt = loginResult.expiresAt;
                autoOkPacket.token = loginResult.token;
                Protocol::SendPacket(session, Protocol::PacketId::S2C_AuthOk, autoOkPacket);
            });
        });
}

void PacketHandler::RegisterCS2ResumeHandleFunc(DbWorker* dbWorker)
{
    network::PacketDispatcher::Register<Protocol::C2S_Resume>(Protocol::PacketId::C2S_Resume,
        [dbWorker](std::shared_ptr<network::Session>& session, const Protocol::C2S_Resume& resumePacket)
        {
            dbWorker->Enqueue([session, token = resumePacket.token](AuthService& auth){
                
                if(session == nullptr)
                    return;

                uint64 userId = 0;
                AuthFailReason authFailReason;

                bool resumeSuccessed = auth.Resume(token, userId, authFailReason);

                if(resumeSuccessed == false)
                {
                    Protocol::S2C_AuthFail authfailPacket;
                    authfailPacket.reason = static_cast<uint16>(authFailReason);
                    Protocol::SendPacket(session, Protocol::PacketId::S2C_AuthFail, authfailPacket);
                    return;
                }

                {
                    auto clientSession = std::static_pointer_cast<ClientSession>(session);
                    clientSession->SetUserId(userId);
                }

                //  TODO
                Protocol::S2C_AuthOk autoOkPacket;
                autoOkPacket.userId = userId;
                autoOkPacket.token = "";
                autoOkPacket.expiresAt = 0;
                Protocol::SendPacket(session, Protocol::PacketId::S2C_AuthOk, autoOkPacket);
            });
        });
}