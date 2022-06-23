char DATA_START[128] = "150 Start transfering data on the data channel"; 
char DATA_COMPLETED[128] = "226 Successfully sent/receive";

struct file_transfer_param {
	int cfd;
	int dfd;
	char file_path[1024];
	int user_id;
};

int upload(char* file_path);
int download(struct file_transfer_param param);