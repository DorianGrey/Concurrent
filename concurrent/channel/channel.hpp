#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <type_traits>

namespace concurrent
{
    /** \brief This is the basic channel class. It provides an internal message-queue and a stream-like
    *         message add/remove.
    *  \param MsgType Type of the messages that are exchanged between the communications partners.
    *  \param Storage Possibly custom container class. 
    *         Note that any container you use has to implement either the queue policy (here: push(T),
    *         pop(), front(), empty() ) directly or has to adopt the queue internals, like shown in
    *         http://www.cplusplus.com/reference/stl/queue/ .
    *         I.e., by default, deque and list are implementing this, like
    *         template < class T, class Container = std::deque<T> > class queue
    *         template < class T, class Container = std::list<T> > class queue .
    *  \param OptArgs Optional arguments that are handled over to the storage class.
    */
    template<typename MsgType, template<typename, typename...> class Storage = std::queue, typename... OptArgs>
    class channel
    {
        static_assert( std::is_copy_constructible<MsgType>::value, "The message type requires to be copy-constructible!" );

        public:
            channel(void) {}
            ~channel() {}

            /**
            Block 1: Adding messages to the queue.
            */


            /** \brief Stream-style function that adds a message to the message-queue. Message is taken as reference.
            *
            * \param msg const MsgType& Message to be added to the queue.
            * \return void
            *
            * \note Function blocks until thread-safe access to the queue is possible.
            */
            void operator<<(const MsgType& msg)
            {
                std::unique_lock<std::mutex> lock(this->__accessMutex); // Since std::queue is not thread-safe, we need the lock here.
                this->__msgQueue.push(msg);
                this->__waitCondition.notify_one();
            }

            /** \brief Stream-style function that adds a message to the message-queue. Message is taken as r-value,
            *         useful e.g. for primitive types.
            *
            * \param msg MsgType&& Message to be added to the queue.
            * \return void
            *
            * \note Function blocks until thread-safe access to the queue is possible.
            */
            void operator<<(MsgType&& msg)
            {
                std::unique_lock<std::mutex> lock(this->__accessMutex);
                this->__msgQueue.push(msg);
                this->__waitCondition.notify_one();
            }


            /** \brief Stream-look-alike function that adds a message to the message-queue. Message is taken as reference.
            *
            * \param msg const MsgType& Message to be added to the queue.
            * \return bool "true" if the message could be added to the queue, false otherwise.
            *
            * \note Function returns immediately, message is only added if the queue can be accessed without waiting.
            */
            bool operator<(const MsgType& msg)
            {
                if (this->__accessMutex.try_lock())
                {
                    this->__msgQueue.push(msg);
                    this->__waitCondition.notify_one(); // Notify next available thread.
                    this->__accessMutex.unlock();
                    return true;
                }
                else
                {
                    return false;
                }
            }

            /** \brief Stream-look-alike function that adds a message to the message-queue. Message is taken as r-value,
            *         useful e.g. for primitive types.
            *
            * \param msg const MsgType& Message to be added to the queue.
            * \return bool "true" if the message could be added to the queue, "false" otherwise.
            *
            * \note Function returns immediately, message is only added if the queue can be accessed without waiting.
            */
            bool operator<(MsgType&& msg)
            {
                if (this->__accessMutex.try_lock())
                {
                    this->__msgQueue.push(msg);
                    this->__waitCondition.notify_one(); // Notify next available thread.
                    this->__accessMutex.unlock();
                    return true;
                }
                else
                {
                    return false;
                }
            }

            /**
            Block 2: Fetching messages from the queue.
            */


            /** \brief Stream-like function that takes the next available message from the internal message queue.
            *
            * \param destination MsgType& Variable to write the message to.
            * \return void
            *
            * \note Function blocks until thread-safe access to the queue is possible and at least one message arrived.
            */
            void operator>>(MsgType& destination)
            {
                std::unique_lock<std::mutex> lock(this->__accessMutex); // Since std::queue is not thread-safe, we need the lock here.
                this->__waitCondition.wait(lock, [this]()->bool             // We have to wait for the given condition after getting the lock granted.
                {
                    return !this->__msgQueue.empty();
                }
                );
                destination = this->__msgQueue.front();
                this->__msgQueue.pop();
            }

            /** \brief Stream-look-alike function that takes the next available message from the internal message queue,
            *         if there is one.
            *
            * \param destination MsgType& Variable to write the message to.
            * \return bool true" if the message could be fetched from the queue, false otherwise.
            *
            * \note Function returns immediately, message is only fetched if at least one is present, and no pending lock exists.
            */
            bool operator>(MsgType& destination)
            {
                if (this->__accessMutex.try_lock())
                {
                    bool couldTakeMessage = false;
                    if (!this->__msgQueue.empty())
                    {
                        destination = this->__msgQueue.front();
                        this->__msgQueue.pop();
                        couldTakeMessage = true;
                    }
                    this->__accessMutex.unlock();
                    return couldTakeMessage;
                }
                else
                {
                    return false;
                }
            }

        private:
            // Prohibitions
            channel(const channel& rhs);                /**< Channels must not be copied */
            channel& operator=(const channel& rhs);     /**< Channels must not be assigned to other channels */
            // Parameters
            std::mutex __accessMutex;
            std::condition_variable __waitCondition;

            Storage<MsgType, OptArgs...> __msgQueue;
    };

    /** \brief This is the channel helper class. Since a channel is regularly used in different threads,
    *         a make_chan() function is provided (below) that automatically intends to create a shared
    *         pointer to the channel. Since this would cause the usage of the dereferencing operator any
    *         time the channel gets accessed, I provide the Chan<> class below that offer wrapper functions
    *         to avoid this. Copying it has the same costs as copying the shared_ptr inside, since it
    *         does not have any additional data members. *
    * \param  MsgType Type of the messages that are exchanged between the communications partners.
    * \param  Storage Container to put the messages to.
    * \param  OptArgs Optional arguments that are forwarded to the storage class.
    * \see    Channel<> for details.
    */
    template<typename MsgType, template<typename, typename...> class Storage = std::queue, typename... OptArgs>
    struct chan
    {
            chan(): __me(std::make_shared<channel<MsgType, Storage, OptArgs...>>())  {}
            void operator<<(const MsgType& msg)
            {
                *__me << msg;
            }
            void operator<<(MsgType&& msg)
            {
                *__me << std::forward<MsgType>(msg);
            }
            bool operator<(const MsgType& msg)
            {
                return (*__me < msg);
            }
            bool operator<(MsgType&& msg)
            {
                return (*__me < std::forward<MsgType>(msg));
            }
            void operator>>(MsgType& destination)
            {
                *__me > destination;
            }
            bool operator>(MsgType& destination)
            {
                return *__me > destination;
            }
        private:
            std::shared_ptr< channel<MsgType, Storage, OptArgs...> > __me;
    };

    /** \brief Helper function to automatically create channel-objects that are managed by shared_ptr objects.
    *         It simply creates an instance of the helper class mentioned above.
    *
    * \return Chan<MsgType,Storage> Channel using type MsgType and Storage, managed by the wrapper struct Chan<>.
    *
    */
    template<typename MsgType, template<typename, typename...> class Storage = std::queue, typename... OptArgs>
    inline chan<MsgType, Storage, OptArgs...> make_chan()
    {
        return chan<MsgType, Storage, OptArgs...>();
    }
}



#endif
