#pragma once

/* internal */
#include "PcBuffer.h"

namespace Coral
{

template <class T, size_t tx_depth, size_t rx_depth,
          typename element_t = std::byte,
          std::size_t alignment = sizeof(element_t)>
class FullDuplexBuffer
{
  public:
    using TxBuffer = PcBuffer<tx_depth, element_t, alignment>;
    using RxBuffer = PcBuffer<rx_depth, element_t, alignment>;

    FullDuplexBuffer(bool _auto_service = true)
        : tx(_auto_service), rx(_auto_service)
    {
        /*
         * Attempt to service the writing end whenever data is ready to be
         * written.
         */
        tx.set_data_available([this](TxBuffer *buf) { service_tx(buf); });

        /*
         * Attempt to service the reading end whenever the read buffer has
         * space.
         */
        rx.set_space_available([this](RxBuffer *buf) { service_rx(buf); });
    }

    /*
     * A method that can be polled at runtime if it's useful for hardware
     * resources to be interacted with regularly.
     */
    inline void dispatch(void)
    {
        service_tx(&tx);
        service_rx(&rx);
    }

    inline void service_tx(TxBuffer *buf)
    {
        static_cast<T *>(this)->service_tx_impl(buf);
    }

    inline void service_rx(RxBuffer *buf)
    {
        static_cast<T *>(this)->service_rx_impl(buf);
    }

    TxBuffer tx;
    RxBuffer rx;
};

} // namespace Coral
