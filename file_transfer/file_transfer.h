struct file_transfer_param {
	int cfd;
	int dfd;
	char file_path[1024];
	int user_id;
};

int upload(char* file_path);
int download(struct file_transfer_param param);