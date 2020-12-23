// Circular_Queue Ŭ���� ���ø� �� ���� Ŭ���� ��� ����
// Round-Robin ����� Wear-leveling�� ���� Empty Block ��⿭, Spare Block ��⿭ ����
// Victim Block ó���� ȿ������ ���� Victim Block ��⿭ ����

/*** Circual Queue Ŭ���� ���ø� ���� (����, �Ҹ�, ť ���� ��ȯ, ��� �� ��ȯ) ***/

template <typename data_type, typename element_type>
Circular_Queue<data_type, element_type>::Circular_Queue()
{
	this->queue_array = NULL;
	this->queue_size = 0;
	this->front = this->rear = 0;
}

template <typename data_type, typename element_type>
Circular_Queue<data_type, element_type>::Circular_Queue(data_type queue_size)
{
	this->queue_array = NULL;
	this->queue_size = queue_size + 1; //������ ���� �� ���� �� ��ġ�� ���� �ϹǷ� queue_size��ŭ �����ϱ� ���� + 1
	this->queue_array = new element_type[this->queue_size];

	this->front = this->rear = 0;
}

template <typename data_type, typename element_type>
Circular_Queue<data_type, element_type>::~Circular_Queue()
{
	if (this->queue_array != NULL)
	{
		delete[] this->queue_array;
		this->queue_array = NULL;
	}
}

template <typename data_type, typename element_type>
bool Circular_Queue<data_type, element_type>::is_empty() //���� ���� ����
{
	return (this->front == this->rear); //front�� rear�� ���� ������ �������
}

template <typename data_type, typename element_type>
bool Circular_Queue<data_type, element_type>::is_full() //��ȭ ���� ����
{
	return ((this->rear + 1) % this->queue_size == this->front); //������ ��ġ���� front�� ������� ��ȭ����
}

template <typename data_type, typename element_type>
data_type Circular_Queue<data_type, element_type>::get_count() //ť�� ��� ���� ��ȯ
{
	return (this->rear) - (this->front);
}

/*** Empty Block ��⿭ ��� ���� (���, ����, ����) ***/

inline void Empty_Block_Queue::print() //���
{
	printf("QUEUE(front = %u rear = %u)\n", this->front, this->rear);
	if (this->is_empty() != true) //ť�� ������� ������
	{
		unsigned int i = this->front;

		do {
			i = (i + 1) % (this->queue_size);

			std::cout << "INDEX : " << i << "|| ";
			std::cout << "empty_block_num : " << this->queue_array[i] << std::endl;
			std::cout << "----------------------------------" << std::endl;

			if (i == this->rear) //rear��ġ���� ���� �� ����
				break;
		} while (i != this->front); //ť�� �� ���� ��������
	}
	printf("\n");
}

inline int Empty_Block_Queue::enqueue(empty_block_num src_block_num) //Empty Block ���� �Ҵ�
{
	if (this->is_full() == true) //���� á����
		return FAIL;

	this->rear = (this->rear + 1) % this->queue_size;
	this->queue_array[this->rear] = src_block_num;

	return SUCCESS;
}

inline int Empty_Block_Queue::dequeue(empty_block_num& dst_block_num) //Empty Block ��������
{
	if (this->is_empty() == true) //���������
		return FAIL;

	this->front = (this->front + 1) % this->queue_size;
	dst_block_num = this->queue_array[this->front];

	return SUCCESS;
}

/*** Spare Block ��⿭ ��� ���� ***/

inline Spare_Block_Queue::Spare_Block_Queue(unsigned int queue_size) : Circular_Queue<unsigned int, spare_block_num>(queue_size)
{
	/***
		< Round-Robin Based Spare Block Table�� ���� ���� read_index ó�� ��� >

		1) ���� �÷��� �޸� �����ϰ� ���ο� �÷��� �޸� �Ҵ� ��
		2) Bootloader�� ���� Reorganization�� ���� ���� �� �÷��� �޸𸮸� �����ϰ�, ���ο� �÷��� �޸� �Ҵ� ��
		---
		=> Physical_func�� init�Լ� ���� �� ���� read_index ����
	***/

	this->load_read_index();
}

inline void Spare_Block_Queue::print() //���
{
	if (this->is_full()) //���� ���� �ʾ��� ��� �ҿ����ϹǷ� �о�� �ȵ�
	{
		fprintf(stderr, "ġ���� ���� : �ҿ����� Spare Block Queue\n");
		system("pause");
		exit(1);
	}

	std::cout << "current read_index : " << this->front << std::endl;

	for (unsigned int i = 0; i < this->queue_size; i++)
	{
		std::cout << "INDEX : " << i << "|| ";
		std::cout << "Spare_block_num : " << this->queue_array[i] << std::endl;
	}
}

inline int Spare_Block_Queue::enqueue(spare_block_num src_block_num) //Spare Block ���� �Ҵ�
{
	if (this->is_full()) //���� á����
		return FAIL;

	this->rear = (this->rear + 1) % this->queue_size;
	this->queue_array[this->rear] = src_block_num;

	return SUCCESS;
}

inline int Spare_Block_Queue::dequeue(class FlashMem*& flashmem, spare_block_num& dst_spare_block, unsigned int& dst_read_index) //�� Spare Block, �ش� ����� index ��������
{
	//���� read_index�� ���� read_index ����, Spare Block ��ȣ ���� �� ���� Spare Block ��ġ�� �̵�

	META_DATA* meta_buffer = NULL;

#ifdef DEBUG_MODE
	if (this->is_full()) //���� ���� �ʾ��� ��� �ҿ����ϹǷ� �о�� �ȵ�
	{
		fprintf(stderr, "ġ���� ���� : �ҿ����� Spare Block Queue\n");
		system("pause");
		exit(1);
	}
#endif
	
	unsigned int end_read_index = this->front;
	do {
		SPARE_read(flashmem, (this->queue_array[this->front] * BLOCK_PER_SECTOR), meta_buffer); //PBN * BLOCK_PER_SECTOR == �ش� PBN�� meta ����

		if (meta_buffer->block_state == BLOCK_STATE::SPARE_BLOCK_EMPTY) //��ȿ�ϰ� ����ִ� ����� ��� ����
		{
			dst_spare_block = this->queue_array[this->front]; //Spare Block�� PBN ����
			dst_read_index = this->front; //SWAP�� ���� �ش� PBN�� ���̺� �� index ����

			this->front = (this->front + 1) % this->queue_size;
			this->save_read_index();

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			return SUCCESS;
		}
		else //���� GC�� ���� ó���� ���� ���� ����� ���, �ش� ����� Erase ���� ������ ��� �Ұ�
			this->front = (this->front + 1) % this->queue_size;

		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

	} while (this->front != end_read_index); //�� ���� ��������

	this->save_read_index();

	return FAIL; //���� ��� ���� �� Spare Block�� �����Ƿ�, Victim Block Queue�� ���� GC���� ó���� �����Ͽ��� ��

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (rr_read)\n");
	system("pause");
	exit(1);
}

inline int Spare_Block_Queue::save_read_index()
{
	FILE* rr_read_index_output = NULL;

#ifdef DEBUG_MODE
	if (!(this->is_full())) //���� ���� �ʾ��� ��� �ҿ����ϹǷ� ó���ؼ��� �ȵ�
	{
		fprintf(stderr, "ġ���� ���� : �ҿ����� Spare Block Queue\n");
		system("pause");
		exit(1);
	}
#endif

	if ((rr_read_index_output = fopen("rr_read_index.txt", "wt")) == NULL) //���� + �ؽ�Ʈ���� ���
		return COMPLETE; //�����ص� ��� ����

	fprintf(rr_read_index_output, "%u", this->rear);
	fclose(rr_read_index_output);

	return SUCCESS;
}

inline int Spare_Block_Queue::load_read_index()
{
	FILE* rr_read_index_input = NULL;

	if ((rr_read_index_input = fopen("rr_read_index.txt", "rt")) == NULL) //�б� + �ؽ�Ʈ���� ���
	{
		this->front = 0;
		return COMPLETE;
	}

	fscanf(rr_read_index_input, "%u", &this->front);
	fclose(rr_read_index_input);

	return SUCCESS;
}

/*** Victim Block ��⿭ ��� ���� ***/

inline void Victim_Block_Queue::print() //���
{
	printf("QUEUE(front = %u rear = %u)\n", this->front, this->rear);
	if (this->is_empty() != true) //ť�� ������� ������
	{
		unsigned int i = this->front;

		do {
			i = (i + 1) % (this->queue_size);

			switch (this->queue_array[i].is_logical)
			{
			case true:
				std::cout << "victim_LBN : ";
				break;

			case false:
				std::cout << "victim_PBN : ";
				break;
			}
			std::cout << this->queue_array[i].victim_block_num << std::endl;
			std::cout << "victim_block_invalid_ratio : " << this->queue_array[i].victim_block_invalid_ratio << std::endl;
			std::cout << "----------------------------------" << std::endl;

			if (i == this->rear) //rear��ġ���� ���� �� ����
				break;
		} while (i != this->front); //ť�� �� ���� ��������
	}
	printf("\n");
}

inline int Victim_Block_Queue::enqueue(victim_block_element src_block_element) //����
{
	if (this->is_full()) //���� á����
		return FAIL;

	this->rear = (this->rear + 1) % this->queue_size;
	this->queue_array[this->rear] = src_block_element;

	return SUCCESS;
}

inline int Victim_Block_Queue::dequeue(victim_block_element& dst_block_element) //����
{
	if (this->is_empty()) //���������
		return FAIL;

	this->front = (this->front + 1) % this->queue_size;
	dst_block_element = this->queue_array[this->front];

	return SUCCESS;
}