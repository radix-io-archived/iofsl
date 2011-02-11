class GetAttrSM
{
    public:
        GetAttrSM()
        {
        }

        /* in all states, run e.check() ... if exception jump to error state */

        /* step 1 */
        void postOpenConnection();
        void waitOpenConnection();

        /* step 3 write */
        /* imead. local complete */
        void postEncodeRequest();
        void waitEncodeRequest();

        /* step 4 */
        void postFlushRequest();
        void waitFlushRequest();

        /* setp 5 read */
        void postDecodeResponse();
        void waitDecodeResponse();

        /* step 2 */
        void postWaitResponse();
        void waitWaitResponse();

        void postSMErrorState();
    proteced:
        addr;
};
