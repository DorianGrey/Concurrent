#ifndef __CONCURRENT_QUEUE_HPP__
#define __CONCURRENT_QUEUE_HPP__

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace concurrent
{
    /** \brief This is the basic concurrent queue class. It provides a stream-like
     *         message add/remove. Most of these functions are borrowed from a former project of mine,
     *         https://github.com/DorianGrey/Channel/ , which was a purpose to build golang-channels 
     *         in C++.
     *  \param MsgType Type of the messages that are exchanged between the communications partners.
     *  \param Storage Container class.
     *         Note that any container you use has to implement either the queue policy (here: push(T),
     *         pop(), front(), empty() ) directly or has to adopt the queue internals, like shown in
     *         http://www.cplusplus.com/reference/stl/queue/ .
     *         Also, using std::swap on it should be supported.
     *         I.e., by default, deque and list are implementing this, like
     *         template < class T, class Container = std::deque<T> > class queue
     *         template < class T, class Container = std::list<T> > class queue .
     *  \param OptArgs Optional arguments that are getting passed to the storage template declaration,
     *         e.g. for a custom allocator.
     */
    template<typename MsgType, template<typename...> class Storage, typename... OptArgs>
    class queue
    {
        public:
            queue(){};
            ~queue(){};

            /** \brief Stream-style function that adds a message to the queue. Message is taken as reference.
            *
            * \param msg const MsgType Message to be added to the queue.
            * \return *this
            *
            * \note Function blocks until thread-safe access to the queue is possible.
            */
            queue& operator<<(const MsgType& msg)
            {
                std::unique_lock<std::mutex> lock(this->__accessMutex); // Since std::queue is not thread-safe, we need the lock here.
                this->__storage.push(msg);
                this->__waitCondition.notify_one();
                return *this;
            }

            /** \brief Stream-style function that adds a message to the message-queue. Message is taken as r-value.
            *
            * \param msg MsgType&& Message to be added to the queue.
            * \return *this
            *
            * \note Function blocks until thread-safe access to the queue is possible.
            */
            queue& operator<<(MsgType&& msg)
            {
                std::unique_lock<std::mutex> lock(this->__accessMutex);
                this->__storage.push(msg);
                this->__waitCondition.notify_one();
                return *this;
            }

            /** \brief Stream-like function that takes the next available message from the internal message queue.
             *
             * \param destination MsgType& Variable to write the message to.
             * \return *this
             *
             * \note Function blocks until thread-safe access to the queue is possible and at least one message arrived.
             */
            queue& operator>>(MsgType& destination)
            {
                std::unique_lock<std::mutex> lock(this->__accessMutex); // Since std::queue is not thread-safe, we need the lock here.
                this->__waitCondition.wait(lock, [this]()->bool         // We have to wait for the given condition after getting the lock granted.
                {
                    return !this->__storage.empty();
                });
                destination = this->__storage.front();
                this->__storage.pop();
                return *this;
            }

            /** \brief Second option to retrieve something from the list - is a little shorter for the caller
             *
             */
            MsgType pop()
            {
                std::unique_lock<std::mutex> lock(this->__accessMutex); // Since std::queue is not thread-safe, we need the lock here.
                this->__waitCondition.wait(lock, [this]()->bool         // We have to wait for the given condition after getting the lock granted.
                {
                    return !this->__storage.empty();
                });
                auto destination = this->__storage.front();
                this->__storage.pop();
                return destination;
            }

            void clear()
            {
                std::unique_lock<std::mutex> lock(this->__accessMutex);
                Storage<MsgType, OptArgs...> empty; // empty storage
                std::swap(this->__storage, empty);
            }

        private:
            // Prohibitions
            queue(const queue& rhs);                /**< queues must not be copied */
            queue& operator=(const queue& rhs);     /**< queues must not be assigned to other channels */
            // Parameters
            mutable std::mutex __accessMutex;
            std::condition_variable __waitCondition;
            Storage<MsgType, OptArgs...> __storage;

    };

    // Not usable yet in VS 2012, may available somewhere else
    /*
    template<typename MsgType, typename... OptArgs>
    using default_queue = queue<MsgType, std::queue, OptArgs...>;
    */

}



#endif // !__CONCURRENT_QUEUE_HPP__
