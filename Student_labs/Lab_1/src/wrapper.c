#include "wrapper.h"

//Big note! If you think the arguments makes no sense, you are allowed to change them, as long as the basic functionality is kept
//In case you run in to blocking issues with reading from the queue, the option O_NONBLOCK is a hot tip

int MQcreate (mqd_t * mq, const char * name)
{
 	//Should create a new messagequeue, use mq as reference pointer so you can reach the handle from anywhere
	//Should return 1 on success and 0 on fail


	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAX_SIZE;


    mqd_t new_mq = mq_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    if (new_mq == (mqd_t)-1) {
        perror("MQCreate");
        fprintf(stderr, "Error code: %d\n", errno);
        return 0;
    }

    *mq = new_mq;
    return 1;
}
int MQconnect (mqd_t * mq, const char * name)
{
    /* Connects to an existing mailslot for writing Uses mq as reference pointer, so that you can 	reach the handle from anywhere*/
    /* Should return 1 on success and 0 on fail*/

	//mq_open returns -1 on error
    mqd_t new_mq = mq_open(name, O_RDWR);
    if (new_mq == (mqd_t)-1) {
    	perror("MQConnect");
		fprintf(stderr, "Error code: %d\n", errno);
        return 0;
    }
    *mq = new_mq;
    return 1;
}

ssize_t MQread (mqd_t mq, void * buffer, size_t buffer_length)
{

    /* Read a msg from a mailslot, return nr Uses mq as reference pointer, so that you can 		reach the handle from anywhere */
    /* should return number of bytes read              */
    ssize_t l = mq_receive(mq, buffer, buffer_length, NULL);

    if (l == -1) {
    	perror("MQRead");
		fprintf(stderr, "Error code: %d\n", errno);
    }

    return l;
}

ssize_t MQwrite (mqd_t mq, const void * data, size_t data_length)
{
    /* Write a msg to a mailslot, return nr Uses mq as reference pointer, so that you can 	     reach the handle from anywhere*/
    /* should return number of bytes read         */
	return mq_send(mq, data, data_length, 0);
}

int MQclose(mqd_t mq)
{
    /* close a mailslot, returning whatever the service call returns Uses mq as reference pointer, so that you can
    reach the handle from anywhere*/
    /* Should return 1 on success and 0 on fail*/

	//mq_close return 0 on success
	if (mq_close(mq) == 0) {
		return 1;
	} else {
		return 0;
	}
}



