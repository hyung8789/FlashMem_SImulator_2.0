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
Circular_Queue<data_type, element_type>::Circular_Queue(data_type queue_size) //�Ҵ� ũ�� - 1���� ���Ҹ� ���� ����, ���� ���� �� + 1 ũ��� ����
{
	this->queue_array = NULL;
	this->queue_size = queue_size;
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
	FILE* ebq_output = NULL;

	if ((ebq_output = fopen("ebq_output.txt", "wt")) == NULL) //���� + �ؽ�Ʈ���� ���
		return;

	printf("QUEUE(front = %u rear = %u)\n", this->front, this->rear);
	if (!(this->is_empty())) //ť�� ������� ������
	{
		unsigned int i = this->front;

		do {
			i = (i + 1) % (this->queue_size);

			fprintf(ebq_output, "INDEX : %u\n", i);
			fprintf(ebq_output, "empty_block_num : %u\n", this->queue_array[i]);
			fprintf(ebq_output, "----------------------------------\n");

			std::cout << "INDEX : " << i << std::endl;
			std::cout << "empty_block_num : " << this->queue_array[i] << std::endl;
			std::cout << "----------------------------------" << std::endl;

			if (i == this->rear) //rear��ġ���� ���� �� ����
				break;
		} while (i != this->front); //ť�� �� ���� ��������
	}
	printf("\n");

	fclose(ebq_output);
	printf(">> ebq_output.txt\n");
	system("notepad ebq_output.txt");
}

inline int Empty_Block_Queue::enqueue(empty_block_num src_block_num) //Empty Block ����
{
	if (this->is_full()) //���� á����
		return FAIL;

	this->rear = (this->rear + 1) % this->queue_size;
	this->queue_array[this->rear] = src_block_num;

	return SUCCESS;
}

inline int Empty_Block_Queue::dequeue(empty_block_num& dst_block_num) //Empty Block ��������
{
	if (this->is_empty()) //���������
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

	this->init_mode = true;
	this->load_read_index();
}

inline void Spare_Block_Queue::print() //���
{
#ifdef DEBUG_MODE
	if (this->init_mode) //���� ���� �ʾ��� ��� �ҿ����ϹǷ� �о�� �ȵ�
	{
		fprintf(stderr, "ġ���� ���� : �ҿ����� Spare Block Queue\n");
		system("pause");
		exit(1);
	}
#endif

	FILE* sbq_output = NULL;

	if ((sbq_output = fopen("sbq_output.txt", "wt")) == NULL) //���� + �ؽ�Ʈ���� ���
		return;

	printf("QUEUE(front = %u rear = %u)\n", this->front, this->rear);

	for (unsigned int i = 1; i < this->queue_size; i++) //0�� �ε��� ������� ����
	{
		fprintf(sbq_output, "INDEX : %u\n", i);
		fprintf(sbq_output, "spare_block_num : %u\n", this->queue_array[i]);
		fprintf(sbq_output, "----------------------------------\n");
		std::cout << "INDEX : " << i << std::endl;
		std::cout << "spare_block_num : " << this->queue_array[i] << std::endl;
		std::cout << "----------------------------------" << std::endl;
	}

	fclose(sbq_output);
	printf(">> sbq_output.txt\n");
	system("notepad sbq_output.txt");
}

inline int Spare_Block_Queue::enqueue(spare_block_num src_block_num) //Spare Block ����
{
	if (!(this->init_mode)) //���� á�� ��� �� �̻� ��� �Ұ�
		return FAIL;

	this->rear = (this->rear + 1) % this->queue_size;
	this->queue_array[this->rear] = src_block_num;

	if (this->is_full())
	{
		this->init_mode = false; //Spare Block ��⿭�� SWAP �۾��� ���� ����
		return COMPLETE;
	}

	return SUCCESS;
}

inline int Spare_Block_Queue::dequeue(class FlashMem*& flashmem, spare_block_num& dst_spare_block, unsigned int& dst_read_index) //�� Spare Block, �ش� ����� index ��������
{
	//���� read_index�� ���� read_index ����, Spare Block ��ȣ ���� �� ���� Spare Block ��ġ�� �̵�

	META_DATA* meta_buffer = NULL;

#ifdef DEBUG_MODE
	if (this->init_mode)
	{
		fprintf(stderr, "ġ���� ���� : �ʱ�ȭ ���� ���� Spare Block Queue\n");
		system("pause");
		exit(1);
	}
#endif
	
	unsigned int end_read_index = this->front;
	do {
		this->front = (this->front + 1) % this->queue_size;

		if (this->front == 0) //0�� �ε��� ��ġ�� ������� �����Ƿ� ���� �ʵ��� �ؾ� �Ѵ�.
			this->front = (this->front + 1) % this->queue_size;

		SPARE_read(flashmem, (this->queue_array[this->front] * BLOCK_PER_SECTOR), meta_buffer); //PBN * BLOCK_PER_SECTOR == �ش� PBN�� meta ����
		
		if (meta_buffer->get_block_state() == BLOCK_STATE::SPARE_BLOCK_EMPTY) //��ȿ�ϰ� ����ִ� ����� ��� ����
		{
			dst_spare_block = this->queue_array[this->front]; //Spare Block�� PBN ����
			dst_read_index = this->front; //SWAP�� ���� �ش� PBN�� ���̺� �� index ����

			this->save_read_index();

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			return SUCCESS;
		}
		//���� GC�� ���� ó���� ���� ���� ����� ���, �ش� ����� Erase ���� ������ ��� �Ұ�

		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

	} while (this->front != end_read_index); //�� ���� �� ������ (Spare Block ��⿭�� �׻� ���� �� �־�� �Ѵ�)

	this->save_read_index();

	return FAIL; //���� ��� ���� �� Spare Block�� �����Ƿ�, Victim Block Queue�� ���� GC���� ó���� �����Ͽ��� ��

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (rr_read)\n");
	system("pause");
	exit(1);
}

inline void Spare_Block_Queue::manual_init(unsigned int spare_block_size) //���� �� ����
{
	if (this->queue_array == NULL) //queue_array�� ���� �� ������ ���� ���� �Ҵ�Ǿ�� �Ѵ�.
		goto MANUAL_INIT_ERR;

	switch (this->init_mode)
	{
	case true:
		this->init_mode = false;
		this->rear = spare_block_size;
		break;

	case false: //�̹� init_mode�� �ƴ� ���, �߸� ȣ��
		goto MANUAL_INIT_ERR;
	}

	return;

MANUAL_INIT_ERR:
	fprintf(stderr, "ġ���� ���� : Spare Block ��⿭ ���� �� ���� ���� �߻� (manual_init)\n");
	system("pause");
	exit(1);
}

inline int Spare_Block_Queue::save_read_index()
{
	FILE* rr_read_index_output = NULL;

	if ((rr_read_index_output = fopen("rr_read_index.txt", "wt")) == NULL) //���� + �ؽ�Ʈ���� ���
		return COMPLETE; //�����ص� ��� ����

	fprintf(rr_read_index_output, "%u", this->front);
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
	FILE* vbq_output = NULL;

	if ((vbq_output = fopen("vbq_output.txt", "wt")) == NULL) //���� + �ؽ�Ʈ���� ���
		return;

	printf("QUEUE(front = %u rear = %u)\n", this->front, this->rear);
	if (!(this->is_empty())) //ť�� ������� ������
	{
		unsigned int i = this->front;

		do {
			i = (i + 1) % (this->queue_size);

			switch (this->queue_array[i].is_logical)
			{
			case true:
				fprintf(vbq_output, "victim_LBN : ");
				std::cout << "victim_LBN : ";
				break;

			case false:
				fprintf(vbq_output, "victim_PBN : ");
				std::cout << "victim_PBN : ";
				break;
			}

			fprintf(vbq_output, "%u\n", this->queue_array[i].victim_block_num);
			std::cout << this->queue_array[i].victim_block_num << std::endl;
			
			switch (this->queue_array[i].proc_state)
			{
			case VICTIM_BLOCK_PROC_STATE::UNLINKED:
				fprintf(vbq_output, "VICTIM_BLOCK_PROC_STATE::UNLINKED\n");
				std::cout << "VICTIM_BLOCK_PROC_STATE::UNLINKED" << std::endl;
				break;

			case VICTIM_BLOCK_PROC_STATE::SPARE_LINKED:
				fprintf(vbq_output, "VICTIM_BLOCK_PROC_STATE::SPARE_LINKED\n");
				std::cout << "VICTIM_BLOCK_PROC_STATE::SPARE_LINKED" << std::endl;
				break;

			case VICTIM_BLOCK_PROC_STATE::UNPROCESSED_FOR_MERGE:
				fprintf(vbq_output, "VICTIM_BLOCK_PROC_STATE::UNPROCESSED_FOR_MERGE\n");
				std::cout << "VICTIM_BLOCK_PROC_STATE::UNPROCESSED_FOR_MERGE" << std::endl;
				break;
			}
			fprintf(vbq_output, "----------------------------------\n");
			std::cout << "----------------------------------" << std::endl;

			if (i == this->rear) //rear��ġ���� ���� �� ����
				break;
		} while (i != this->front); //ť�� �� ���� ��������
	}
	printf("\n");

	fclose(vbq_output);
	printf(">> vbq_output.txt\n");
	system("notepad vbq_output.txt");
}

inline int Victim_Block_Queue::enqueue(victim_block_element src_block_element) //Victim Block ����
{
	if (this->is_full()) //���� á����
		return FAIL;

	this->rear = (this->rear + 1) % this->queue_size;
	this->queue_array[this->rear] = src_block_element;

	return SUCCESS;
}

inline int Victim_Block_Queue::dequeue(victim_block_element& dst_block_element) //Victim Block ��������
{
	if (this->is_empty()) //���������
		return FAIL;

	this->front = (this->front + 1) % this->queue_size;
	dst_block_element = this->queue_array[this->front];

	return SUCCESS;
}