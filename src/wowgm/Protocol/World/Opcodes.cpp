#include "Opcodes.hpp"
#include "WorldSocket.hpp"
#include "AuthentificationPackets.hpp"

namespace wowgm::protocol::world
{
    class WorldPacket;

    template <class PacketClass, bool (WorldSocket::*Handler)(PacketClass&)>
    class PacketHandler : public ServerOpcodeHandler
    {
    public:
        explicit PacketHandler()
            : ServerOpcodeHandler() { }

        bool Call(WorldSocket* session, WorldPacket& packet) const override
        {
            PacketClass nicePacket(std::move(packet));
            nicePacket.Read();
            return (session->*Handler)(nicePacket);
        }
    };

    namespace detail
    {
        template <typename T>
        struct get_packet_class_t;

        template <typename PacketClass>
        struct get_packet_class_t<bool(WorldSocket::*)(PacketClass&)> {
            using type = PacketClass;
        };
    }

    template<typename Handler, Handler HandlerFunction>
    void OpcodeTable::DefineHandler(Opcode opcode)
    {
        if (opcode == Opcode::NULL_OPCODE || int(opcode) == 0)
            return;

        _opcodeHandlers[static_cast<KeyType>(opcode)] = new PacketHandler<typename detail::get_packet_class_t<Handler>::type, HandlerFunction>();
    }

    void OpcodeTable::Initialize()
    {
#define DEFINE_HANDLER(opcode, handler) DefineHandler<decltype(handler), handler>(Opcode::opcode);

        DEFINE_HANDLER(SMSG_AUTH_CHALLENGE, &WorldSocket::HandleAuthChallenge);
        DEFINE_HANDLER(SMSG_AUTH_RESPONSE, &WorldSocket::HandleAuthResponse);

#undef DEFINE_HANDLER
    }
}
