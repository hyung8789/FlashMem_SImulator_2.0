#include "Build_Options.h"

//��ɾ� ��� ���, �÷��� �޸��� ����, ���� ���̺� �ҷ����� �� ���丮�� �뷮�� ����ϴ� ���� �Լ� ����

void VICTIM_BLOCK_INFO::clear_all() //Victim Block ������ ���� ��� ���� ����ü �ʱ�ȭ
{
	this->is_logical = true;

	//���� �� ���� ������ �ʱ�ȭ
	this->victim_block_num = DYNAMIC_MAPPING_INIT_VALUE;
	this->victim_block_invalid_ratio = -1;

	this->proc_status = VICTIM_BLOCK_PROC_STATUS::UNLINKED;
}

void VARIABLE_FLASH_INFO::clear_all() //��� �ʱ�ȭ
{
	/*** Variable Information ***/
	this->written_sector_count = 0;
	this->invalid_sector_count = 0;

	//Global �÷��� �޸� �۾� ī��Ʈ
	this->flash_write_count = 0;
	this->flash_erase_count = 0;
	this->flash_read_count = 0;
}

void VARIABLE_FLASH_INFO::clear_trace_info() //Global Flash �۾� Ƚ�� ������ ���� ���� �ʱ�ȭ
{
	this->flash_write_count = 0;
	this->flash_erase_count = 0;
	this->flash_read_count = 0;
}

void TRACE_INFO::clear_all() //trace�� ���� ���� �ʱ�ȭ
{
	this->write_count = 0;
	this->read_count = 0;
	this->erase_count = 0;
}

FlashMem::FlashMem()
{
	/*** Fixed information ***/
	this->f_flash_info.flashmem_size = 0; //�÷��� �޸��� MB ũ��
	this->f_flash_info.block_size = 0; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� ��ü ����� ���� (Spare Block ����)
	this->f_flash_info.sector_size = 0; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� ��ü ������ ���� (Spare Block ����)
	//for ftl algorithm
	this->f_flash_info.data_only_storage_byte = 0; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� �� byte ũ�� ��ȯ (Spare Area ����)
	this->f_flash_info.storage_byte = 0; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� �� byte ũ�� (Spare Area ����)
	this->f_flash_info.spare_area_byte = 0; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� ��ü Spare Area�� byte ũ��
	this->f_flash_info.spare_block_size = 0; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� �ý��ۿ��� �����ϴ� Spare Block ����
	this->f_flash_info.spare_block_byte = 0; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� �ý��ۿ��� �����ϴ� Spare Block�� �� byte ũ��

	/*** Variable Information ***/
	this->v_flash_info.clear_all();

	/*** ���� ���̺� �ʱ�ȭ ***/
	this->block_level_mapping_table = NULL; //��� ���� ���̺�
	this->log_block_level_mapping_table = NULL; //1 : 2 ��� ���� ���̺� (Log Algorithm)
	this->offset_level_mapping_table = NULL; //������ ���� ���� ���̺�

	this->empty_block_queue = NULL; //�� ��� ��⿭
	this->spare_block_queue = NULL; //Spare ��� ��⿭

	this->victim_block_info.clear_all(); //Victim Block ������ ���� ��� ���� ����ü 
	this->victim_block_queue = NULL; //Victim ��� ��⿭

	this->gc = NULL;

	this->page_trace_info = NULL;
	this->block_trace_info = NULL;

	this->search_mode = SEARCH_MODE::SEQ_SEARCH; //�ʱ� ����Ž�� ���
}

FlashMem::FlashMem(unsigned short megabytes) //megabytes ũ���� �÷��� �޸� ����
{
	/*** Fixed Information ***/
	this->f_flash_info.flashmem_size = megabytes; //�÷��� �޸��� MB ũ��
	this->f_flash_info.block_size = this->f_flash_info.flashmem_size * MB_PER_BLOCK; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� ��ü ����� ���� (Spare Block ����)
	this->f_flash_info.sector_size = this->f_flash_info.block_size * BLOCK_PER_SECTOR; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� ��ü ������ ���� (Spare Block ����)
	//for ftl algorithm
	this->f_flash_info.data_only_storage_byte = this->f_flash_info.sector_size * SECTOR_PER_BYTE; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� �� byte ũ�� ��ȯ (Spare Area ����)
	this->f_flash_info.storage_byte = this->f_flash_info.sector_size * SECTOR_INC_SPARE_BYTE; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� �� byte ũ�� (Spare Area ����)
	this->f_flash_info.spare_area_byte = this->f_flash_info.sector_size * SPARE_AREA_BYTE; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� ��ü Spare Area�� byte ũ��
	this->f_flash_info.spare_block_size = (unsigned int)round(this->f_flash_info.block_size * SPARE_BLOCK_RATIO); //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� �ý��ۿ��� �����ϴ� Spare Block ����
	this->f_flash_info.spare_block_byte = this->f_flash_info.spare_block_size * SECTOR_INC_SPARE_BYTE * BLOCK_PER_SECTOR; //�Ҵ�� �޸� ũ�⿡ �ش��ϴ� �ý��ۿ��� �����ϴ� Spare Block�� �� byte ũ��

	/*** Variable Information ***/
	this->v_flash_info.clear_all();

	/*** ���� ���̺� �ʱ�ȭ ***/
	this->block_level_mapping_table = NULL; //��� ���� ���̺�
	this->log_block_level_mapping_table = NULL; //1 : 2 ��� ���� ���̺� (Log Algorithm)
	this->offset_level_mapping_table = NULL; //������ ���� ���� ���̺�

	this->empty_block_queue = NULL; //�� ��� ��⿭
	this->spare_block_queue = NULL; //Spare ��� ��⿭

	this->victim_block_info.clear_all(); //Victim Block ������ ���� ��� ���� ����ü 
	this->victim_block_queue = NULL; //Victim ��� ��⿭

	/*** ������ �ݷ��� ��ü ���� ***/
	this->gc = new GarbageCollector();

	this->page_trace_info = NULL;
	this->block_trace_info = NULL;

	this->search_mode = SEARCH_MODE::SEQ_SEARCH; //�ʱ� ����Ž�� ���

#ifdef PAGE_TRACE_MODE
	this->page_trace_info = new TRACE_INFO[this->f_flash_info.sector_size]; //��ü ����(������) ���� ũ��� �Ҵ�
	for (unsigned int PSN = 0; PSN < this->f_flash_info.sector_size; PSN++)
		page_trace_info[PSN].clear_all(); //�б�, ����, ����� Ƚ�� �ʱ�ȭ
#endif

#ifdef BLOCK_TRACE_MODE
	this->block_trace_info = new TRACE_INFO[this->f_flash_info.block_size]; //��ü ��� ���� ũ��� �Ҵ�
	for (unsigned int PBN = 0; PBN < this->f_flash_info.block_size; PBN++)
		block_trace_info[PBN].clear_all(); //�б�, ����, ����� Ƚ�� �ʱ�ȭ
#endif
}

FlashMem::~FlashMem()
{
	this->deallocate_table();

	delete this->gc;
	this->gc = NULL;

	if (this->empty_block_queue != NULL)
	{
		delete this->empty_block_queue;
		this->empty_block_queue = NULL;
	}

	if (this->victim_block_queue != NULL)
	{
		delete this->victim_block_queue;
		this->victim_block_queue = NULL;
	}

#ifdef PAGE_TRACE_MODE
	delete[] this->page_trace_info;
	this->page_trace_info = NULL;
#endif
#ifdef BLOCK_TRACE_MODE
	delete[] this->block_trace_info;
	this->block_trace_info = NULL;
#endif
}

void FlashMem::bootloader(FlashMem*& flashmem, MAPPING_METHOD& mapping_method, TABLE_TYPE& table_type) //Reorganization process from initialized flash memory storage file
{
	/***
	- Reorganization process from initialized flash memory storage file :

	1) ������ ������ �÷��� �޸��� ũ��, ���� ���, ���̺� Ÿ�� �ε��Ͽ� ���ο� �÷��� �޸� ��ü ����
		1-1) �̿� ���� ���� ���̺� ĳ��
	
	2) ������ ���丮�� ���Ϸκ��� ������ ���丮�� ������ ���� (��ϵ� ���� ��, ��ȿȭ�� ���� ��)
		2-1) ������ ���丮�� ������ ���� �� Victim Block ť �� ������ ���� ��� ��� �� ������ �ʿ��� ������尡 ����� ũ�� ������
			 ������ ���丮�� ������ �����ϸ鼭 ���� ��ȿȭ�� ���� ���(valid_block == false)�� Victim Block ť�� ����
		2-2) ���� �Ϸ�� ������ ���丮�� ������ ���� GC�� ���� ������ ��ȿ�� �Ӱ谪 ����
	***/

	FILE* volume = NULL;  //������ �÷��� �޸��� ���� (MB������ ũ��, ���� ���, ���̺� Ÿ��)�� �����ϱ� ���� ���� ������

	META_DATA* meta_buffer = NULL; //���� ����(������)�� meta ����
	META_DATA** block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ����
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	if (flashmem == NULL)
	{
		if ((volume = fopen("volume.txt", "rt")) == NULL) //�б� + �ؽ�Ʈ���� ���
			return;
		else
		{
			while (1)
			{
				system("cls");
				int input = 0;
				std::cout << "Already initialized continue?" << std::endl;
				std::cout << "0: Ignore, 1: Load" << std::endl;
				std::cout << ">>";
				std::cin >> input;
				if (input == 0)
				{
					std::cin.clear(); //������Ʈ�� �ʱ�ȭ
					std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���
					return;
				}
				else if (input == 1) //������ ����� �÷��� �޸��� �뷮 �� ���̺� Ÿ�� �ҷ��ͼ� �� �Ҵ�
				{
					unsigned short megabytes = 0;
					fscanf(volume, "%hd\n", &megabytes);
					flashmem = new FlashMem(megabytes); //�뷮 �����Ͽ� ����
					fscanf(volume, "%d\n", &mapping_method); //���� ��� ����
					fscanf(volume, "%d", &table_type); //���̺� Ÿ�� ����
					std::cin.clear(); //������Ʈ�� �ʱ�ȭ
					std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���
					break;
				}
				else
				{

					std::cin.clear(); //������Ʈ�� �ʱ�ȭ
					std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���
				}
			}
		}

		/*** ���� ���̺� ĳ�� ***/
		flashmem->load_table(mapping_method);

		/*** 
			1) ������ ���丮�� ���Ϸκ��� ������ ���丮�� ������ ���� (��ϵ� ���� ��, ��ȿȭ�� ���� ��)
			2) ������ ���丮�� ������ ���� Victim Block ���� ���� ������ ��ȿ�� �Ӱ谪 ����
			----
			!! ������ ���丮�� ������ ���� �� Victim Block ť �� ������ ���� ��� ��� �� ������ �ʿ��� ������尡 ����� ũ�� ������
			������ ���丮�� ������ �����ϸ鼭 ���� ��ȿȭ�� ���� ���(valid_block == false)�� Victim Block ť�� �����Ѵ�.
		***/
		f_flash_info = flashmem->get_f_flash_info();

		/*** GC�� ���� Victim Block ��⿭, Empty Block ��⿭ ���� ***/
		switch (mapping_method)
		{
		case MAPPING_METHOD::NONE:
			break;

		default:
			flashmem->victim_block_queue = new Victim_Block_Queue(f_flash_info.block_size);

			if (table_type == TABLE_TYPE::DYNAMIC) //Static Table�� ��� ���� �߻� �� �� ��� �Ҵ��� �ʿ� �����Ƿ�, Empty Block Queue�� ������� �ʴ´�.
				flashmem->empty_block_queue = new Empty_Block_Queue(f_flash_info.block_size - f_flash_info.spare_block_size);
			break;
		}

		for (unsigned int PBN = 0; PBN < f_flash_info.block_size; PBN++)
		{
			printf("Reorganizing...(%.1f%%)\r", ((float)PBN / (float)(f_flash_info.block_size - 1)) * 100);
			SPARE_reads(flashmem, PBN, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��

			switch (block_meta_buffer_array[0]->block_state)
			{
			case BLOCK_STATE::NORMAL_BLOCK_INVALID:
			case BLOCK_STATE::SPARE_BLOCK_INVALID:
				update_victim_block_info(flashmem, false, VICTIM_BLOCK_PROC_STATUS::SPARE_LINKED, PBN, block_meta_buffer_array, mapping_method, table_type);
				break;

			case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //����ִ� �Ϲ� ����̸� Empty Block ��⿭�� �߰� (Dyamic Table)
				if (table_type == TABLE_TYPE::DYNAMIC)
					flashmem->empty_block_queue->enqueue(PBN);
				break;

			default:
				break;
			}

			/*** �о���� meta ������ ���� ������ �÷��� �޸� ���� ���� ***/
			if (update_v_flash_info_for_reorganization(flashmem, block_meta_buffer_array) != SUCCESS)
				goto WRONG_META_ERR;

			/*** Deallocate block_meta_buffer_array ***/
			if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
				goto MEM_LEAK_ERR;

			/*** ��ȿȭ�� ��� ���� �� Victim Block ť ����, �̿� ���� ť�� ���� �� �� ó�� ***/
			flashmem->gc->scheduler(flashmem, mapping_method, table_type);
		}

		flashmem->gc->RDY_v_flash_info_for_set_invalid_ratio_threshold = true; //��ȿ�� �Ӱ谪 ������ ���� ������ ���丮�� ���� ���� �Ϸ� �˸�

		/*** ���� �Ϸ�� ������ ���丮�� ������ ���� GC�� ���� ������ ��ȿ�� �Ӱ谪 ���� ***/
		flashmem->gc->scheduler(flashmem, mapping_method, table_type);
		flashmem->gc->print_invalid_ratio_threshold();

		printf("Reorganizing Process Success\n");
		system("pause");
	}

	if (volume != NULL)
		fclose(volume);
	return;

WRONG_META_ERR: //�߸��� meta���� ����
	fprintf(stderr, "ġ���� ���� : �߸��� meta ���� (bootloader)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (bootloader)\n");
	system("pause");
	exit(1);
}

void FlashMem::load_table(MAPPING_METHOD mapping_method) //���ι�Ŀ� ���� ���� ���̺� �ε�
{
	/***
		< ���̺� load ���� >
	
		1) Block level(normal or log block) table
		2) Spare Block Queue
		3) Offset level table
	***/

	FILE* table_input = NULL;

	//����� ���� ���̺�κ��� �Ҵ� ����

	if ((table_input = fopen("table.bin", "rb")) == NULL) //�б� + �������� ���
		goto LOAD_ERR;

	switch (mapping_method) //���� ��Ŀ� ���� ĳ���� �����Ҵ� �� �ҷ�����
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		if (this->block_level_mapping_table == NULL && this->spare_block_queue == NULL)
		{
			//��� ���� ���� ���̺�, Spare Block Queue
			this->block_level_mapping_table = new unsigned int[this->f_flash_info.block_size - this->f_flash_info.spare_block_size];
			fread(this->block_level_mapping_table, sizeof(unsigned int), this->f_flash_info.block_size - this->f_flash_info.spare_block_size, table_input);

			this->spare_block_queue = new Spare_Block_Queue(f_flash_info.spare_block_size + 1);
			fread(this->spare_block_queue->queue_array, sizeof(unsigned int), this->f_flash_info.spare_block_size + 1, table_input);
			this->spare_block_queue->manual_init(f_flash_info.spare_block_size); //Spare Block ��⿭�� ���� �� rear ��ġ ���� �� ����
		}
		else
			goto LOAD_ERR;
		break;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ���� (Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (this->log_block_level_mapping_table == NULL && this->spare_block_queue == NULL && this->offset_level_mapping_table == NULL)
		{
			//��� ���� 1:2 ���� ���̺�, Spare Block Queue, ������ ���� ���̺�
			this->log_block_level_mapping_table = new unsigned int* [this->f_flash_info.block_size - this->f_flash_info.spare_block_size]; //row : ��ü PBN�� ��

			for (unsigned int i = 0; i < this->f_flash_info.block_size - this->f_flash_info.spare_block_size; i++)
			{
				log_block_level_mapping_table[i] = new unsigned int[2]; //col : �� ������ ���� PBN1, PBN2�� ��Ÿ��
				//�� ���� ������ �Ҵ��߱� ������ �� ���� ��ü �迭�� ���� �� ����. �� ������ ����
				fread(this->log_block_level_mapping_table[i], sizeof(unsigned int), 2, table_input);
			}

			this->spare_block_queue = new Spare_Block_Queue(f_flash_info.spare_block_size + 1);
			fread(this->spare_block_queue->queue_array, sizeof(unsigned int), this->f_flash_info.spare_block_size + 1, table_input);
			this->spare_block_queue->manual_init(f_flash_info.spare_block_size); //Spare Block ��⿭�� ���� �� rear ��ġ ���� �� ����
			
			this->offset_level_mapping_table = new __int8[this->f_flash_info.block_size * BLOCK_PER_SECTOR];
			fread(this->offset_level_mapping_table, sizeof(__int8), this->f_flash_info.block_size * BLOCK_PER_SECTOR, table_input);
		}
		else
			goto LOAD_ERR;
		break;

	default:
		return;
	}

	fclose(table_input);
	return;

LOAD_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ���̺� �ҷ����� ����\n");
	system("pause");
	exit(1);
}
void FlashMem::save_table(MAPPING_METHOD mapping_method) //���ι�Ŀ� ���� ���� ���̺� ����
{
	/***
		< ���̺� save ���� >
		
		1) Block level(normal or log block) table
		2) Spare Block Queue
		3) Offset level table
	***/

	FILE* table_output = NULL;

	if ((table_output = fopen("table.bin", "wb")) == NULL) //���� + �������� ���
	{
		fprintf(stderr, "table.bin ������ ������� �� �� �����ϴ�. (save_table)");
		return; //���߿� �� ��� �õ� �� �� �����Ƿ�, �����ص� ��� ����
	}

	switch (mapping_method) //���� ��Ŀ� ���� ����
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		//��� ���� ���� ���̺�, Spare Block Queue
		if (this->block_level_mapping_table != NULL && this->spare_block_queue != NULL)
		{
			fwrite(this->block_level_mapping_table, sizeof(unsigned int), this->f_flash_info.block_size - this->f_flash_info.spare_block_size, table_output);
			fwrite(this->spare_block_queue->queue_array, sizeof(unsigned int), this->f_flash_info.spare_block_size + 1, table_output);
		}
		else
			goto SAVE_ERR;
		break;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ���� (Log algorithm - 1:2 Block level mapping with Dynamic Table)
		//��� ���� 1:2 ���� ���̺�, Spare Block Queue, ������ ���� ���̺�
		if (this->log_block_level_mapping_table != NULL && this->spare_block_queue != NULL && this->offset_level_mapping_table != NULL)
		{
			//�� ���� ������ �Ҵ��߱� ������ �� ���� ��ü �迭�� �� �� ����. �� ������ ����
			for (unsigned int i = 0; i < this->f_flash_info.block_size - this->f_flash_info.spare_block_size; i++)
			{
				fwrite(this->log_block_level_mapping_table[i], sizeof(unsigned int), 2, table_output);
			}
			fwrite(this->spare_block_queue->queue_array, sizeof(unsigned int), this->f_flash_info.spare_block_size + 1, table_output);
			fwrite(this->offset_level_mapping_table, sizeof(__int8), this->f_flash_info.block_size * BLOCK_PER_SECTOR, table_output); //������ ���� ���� ���̺� ���
		}
		else
			goto SAVE_ERR;
		break;

	default:
		return;
	}

	fclose(table_output);
	return;

SAVE_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ���̺� ���� ����\n");
	system("pause");
	exit(1);
}

void FlashMem::deallocate_table() //���� ĳ�õ� ��� ���̺� ����
{
	//��� ���� ���̺�
	if (this->block_level_mapping_table != NULL)
	{
		delete[] this->block_level_mapping_table;
		this->block_level_mapping_table = NULL;
	}

	//1 : 2 ��� ���� ���̺� (Log Algorithm)
	if (this->log_block_level_mapping_table != NULL)
	{
		//Spare block ������ ������ ũ�Ⱑ ���̺��� ũ��
		for (unsigned int i = 0; i < this->f_flash_info.block_size - this->f_flash_info.spare_block_size; i++)
		{
			delete[] this->log_block_level_mapping_table[i];
		}
		delete[] this->log_block_level_mapping_table;
		this->log_block_level_mapping_table = NULL;
	}

	//������ ���� ���� ���̺�
	if (this->offset_level_mapping_table != NULL)
	{
		delete[] this->offset_level_mapping_table;
		this->offset_level_mapping_table = NULL;
	}

	//Spare Block Queue
	if (this->spare_block_queue != NULL)
	{
		delete this->spare_block_queue;
		this->spare_block_queue = NULL;
	}
}

void FlashMem::disp_command(MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //Ŀ�ǵ� ��ɾ� ���
{
	switch (mapping_method)
	{
	case MAPPING_METHOD::NONE:
		system("cls");
		std::cout << "====================================================================" << std::endl;
		std::cout << "	�÷��� �޸� �ùķ����� - Non-FTL" << std::endl;
		std::cout << "====================================================================" << std::endl;
		std::cout << " init x �Ǵ� i x - x MB ��ŭ �÷��� �޸� ���� " << std::endl;
		std::cout << " read PSN �Ǵ� r PSN - ���� ������ ������ �б�" << std::endl;
		std::cout << " write PSN data �Ǵ� w PSN data - ���� ������ data ���" << std::endl;
		std::cout << " erase PBN �Ǵ� e PBN - ���� ����� ������ ����" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " change - ���� ��� ����" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " clrglobalcnt - �÷��� �޸��� ��ü Read, Write, Erase Ƚ�� �ʱ�ȭ  " << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " pbninfo PBN - �ش� ���� ����� ��� ������ meta���� ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " info - �÷��� �޸� ���� ���" << std::endl;
		std::cout << " exit - ����" << std::endl;
		std::cout << "====================================================================" << std::endl;
		break;

	case MAPPING_METHOD::BLOCK:
		system("cls");
		std::cout << "====================================================================" << std::endl;
		std::cout << "	�÷��� �޸� �ùķ����� - Block Mapping ";

		if(table_type == TABLE_TYPE::STATIC) std::cout << "(Static Table)" << std::endl;
		else std::cout << "(Dynamic Table)" << std::endl;
		
		std::cout << "====================================================================" << std::endl;
		std::cout << " init x �Ǵ� i x - x MB ��ŭ �÷��� �޸� ���� " << std::endl;
		std::cout << " read LSN �Ǵ� r LSN - �� ������ ������ �б�" << std::endl;
		std::cout << " write LSN data �Ǵ� w LSN data - �� ������ data ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " change - ���� ��� ����" << std::endl;
		std::cout << " print - ���� ���̺� ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " searchmode - ��� ���� �� �������� ã�� ���� �˰��� ����" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " trace - trace���Ϸκ��� ���� ���� ����  " << std::endl;
		std::cout << " clrglobalcnt - �÷��� �޸��� ��ü Read, Write, Erase Ƚ�� �ʱ�ȭ  " << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		if(table_type == TABLE_TYPE::DYNAMIC)
			std::cout << " ebqprint - Empty Block ��⿭ ���  " << std::endl;
		std::cout << " sbqprint - Spare Block ��⿭ ���  " << std::endl;
		std::cout << " vbqprint - Victim Block ��⿭ ���  " << std::endl;
		std::cout << " lbninfo LBN - �ش� LBN�� ������ ���� ����� ��� ������ meta���� ���" << std::endl;
		std::cout << " pbninfo PBN - �ش� ���� ����� ��� ������ meta���� ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " info - �÷��� �޸� ���� ���" << std::endl;
		std::cout << " exit - ����" << std::endl; //Victim Block ť�� ��� ���� ���� ���� ����(GC�� �۾����� �ƴ� ��)
		std::cout << "====================================================================" << std::endl;
		break;

	case MAPPING_METHOD::HYBRID_LOG:
		system("cls");
		std::cout << "====================================================================" << std::endl;
		std::cout << "	�÷��� �޸� �ùķ����� - Hybrid Mapping ";

		if (table_type == TABLE_TYPE::STATIC) std::cout << "(Static Table)" << std::endl;
		else std::cout << "(Dynamic Table)" << std::endl;
		
		std::cout << "====================================================================" << std::endl;
		std::cout << " init x �Ǵ� i x - x MB ��ŭ �÷��� �޸� ���� " << std::endl;
		std::cout << " read LSN �Ǵ� r LSN - �� ������ ������ �б�" << std::endl;
		std::cout << " write LSN data �Ǵ� w LSN data - �� ������ data ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " change - ���� ��� ����" << std::endl;
		std::cout << " print - ���� ���̺� ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " searchmode - ��� ���� �� �������� ã�� ���� �˰��� ����" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " trace - trace���Ϸκ��� ���� ���� ����  " << std::endl;
		std::cout << " clrglobalcnt - �÷��� �޸��� ��ü Read, Write, Erase Ƚ�� �ʱ�ȭ  " << std::endl;
		//std::cout << " gc - ������ ������ �÷��� �ǽ�  " << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		if (table_type == TABLE_TYPE::DYNAMIC)
			std::cout << " ebqprint - Empty Block ��⿭ ���  " << std::endl;
		std::cout << " sbqprint - Spare Block ��⿭ ���  " << std::endl;
		std::cout << " vbqprint - Victim Block ��⿭ ���  " << std::endl;
		std::cout << " lbninfo LBN - �ش� LBN�� ������ ���� ����� ��� ������ meta���� ���" << std::endl;
		std::cout << " pbninfo PBN - �ش� ���� ����� ��� ������ meta���� ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " info - �÷��� �޸� ���� ���" << std::endl;
		std::cout << " exit - ����" << std::endl; //Victim Block ť�� ��� ���� ���� ���� ����(GC�� �۾����� �ƴ� ��)
		std::cout << "====================================================================" << std::endl;
		break;

	default:
		break;
	}
}

void FlashMem::input_command(FlashMem*& flashmem, MAPPING_METHOD& mapping_method, TABLE_TYPE& table_type) //Ŀ�ǵ� ��ɾ� �Է�
{
	std::string user_input; //����ڷκ��� ��ɾ �� �ٷ� �Է¹޴� ����
	/*** user_input���κ��� �и��ϴ� ���� ***/
	std::string command; //��ɾ�
	char data = NULL; //����ϰ��� �ϴ� ������
	unsigned short megabytes = -1; //�����ϰ��� �ϴ� �÷��� �޸� �뷮
	unsigned int LSN = -1; //�� ���� ��ȣ
	unsigned int LBN = -1; //�� ��� ��ȣ
	unsigned int PBN = -1; //���� ��� ��ȣ
	unsigned int PSN = -1; //���� ���� ��ȣ

	char data_output = NULL; //���� ���ͷκ��� �о���� ������

	std::cout << "��ɾ� >> ";
	std::getline(std::cin, user_input); //�� �ٷ� �Է¹ޱ�
	std::stringstream ss(user_input); //�и�
	ss >> command;

	switch (mapping_method)
	{
	case MAPPING_METHOD::NONE: //���� ������� ����
		if (command.compare("init") == 0 || command.compare("i") == 0) //megabytes ��ŭ �÷��� �޸� ����
		{
			ss >> megabytes;
			if (megabytes <= 0)
			{
				std::cout << "must over 0 megabytes" << std::endl;
				system("pause");
				break;
			}
			else if (megabytes > MAX_CAPACITY_MB)
			{
				std::cout << "�ִ� �Ҵ� ���� ���� �ʰ�" << std::endl;
				system("pause");
				break;
			}

			init(flashmem, megabytes, mapping_method, table_type);
			system("pause");
		}
		else if (command.compare("read") == 0 || command.compare("r") == 0) //���� ������ ������ �б�
		{
			ss >> PSN;
			if (PSN < 0)
			{
				std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
				system("pause");
				break;
			}

			if(Flash_read(flashmem, DO_NOT_READ_META_DATA, PSN, data_output) != FAIL)
			{
				if(data_output != NULL)
					std::cout << data_output << std::endl;
				else
					std::cout << "no data" << std::endl;
			}
			
			system("pause");
			break;

		}
		else if (command.compare("write") == 0 || command.compare("w") == 0) //���� ���Ϳ� data ���
		{
			ss >> PSN;
			ss >> data;
			
			if (PSN < 0 || data == NULL)
			{
				std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
				system("pause");
				break;
			}

			Flash_write(flashmem, DO_NOT_READ_META_DATA, PSN, data);
			system("pause");
		}
		else if (command.compare("erase") == 0 || command.compare("e") == 0) //���� ��� ��ȣ�� �ش�Ǵ� ����� ������ ����
		{
			ss >> PBN;

			if (PBN < 0)
			{
				std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
				system("pause");
				break;
			}
			
			Flash_erase(flashmem, PBN);
			system("pause");
		}
		else if (command.compare("change") == 0) //���� ��� ����
		{
			flashmem->switch_mapping_method(mapping_method, table_type);

			if(flashmem != NULL)
				flashmem->deallocate_table(); //���� ���̺� ����
		}
		else if (command.compare("clrglobalcnt") == 0) //�÷��� �޸��� ��ü Read, Write, Erase Ƚ�� �ʱ�ȭ
		{
			flashmem->v_flash_info.clear_trace_info();
		}
		else if (command.compare("pbninfo") == 0) //PBN meta ���� ���
		{
			ss >> PBN;

			if (PBN < 0)
			{
				std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
				system("pause");
				break;
			}

			print_block_meta_info(flashmem, false, PBN, mapping_method);
			system("pause");
		}
		else if (command.compare("info") == 0) //���� ���
		{
			flashmem->disp_flash_info(flashmem, mapping_method, table_type);
			system("pause");
		}
		else if (command.compare("exit") == 0) //����
		{
			exit(1);
		}
		else if (command.compare("cleaner") == 0)
		{
			system("cleaner.cmd");
		}
		else
		{
			std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
			system("pause");
			break;
		}
		break;

	default: //���, ���̺긮�� ����
		if (command.compare("init") == 0 || command.compare("i") == 0) //megabytes ��ŭ �÷��� �޸� ����
		{
			ss >> megabytes;

			if (megabytes <= 0)
			{
				std::cout << "must over 0 megabytes" << std::endl;
				system("pause");
				break;
			}

			init(flashmem, megabytes, mapping_method, table_type);
			system("pause");
		}
		else if (command.compare("read") == 0 || command.compare("r") == 0) //�� ������ ������ �б�
		{
			ss >> LSN;

			if (LSN < 0)
			{
				std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
				system("pause");
				break;
			}

			FTL_read(flashmem, LSN, mapping_method, table_type);
			system("pause");
			break;

		}
		else if (command.compare("write") == 0 || command.compare("w") == 0) //�� ���Ϳ� data ���
		{
			ss >> LSN;
			ss >> data;

			if (LSN < 0 || data == NULL)
			{
				std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
				system("pause");
				break;
			}

			FTL_write(flashmem, LSN, data, mapping_method, table_type);
			system("pause");
		}
		else if (command.compare("change") == 0) //���� ��� ����
		{
			flashmem->switch_mapping_method(mapping_method, table_type);
			
			if(flashmem != NULL)
				flashmem->deallocate_table(); //���� ���̺� ����
		}
		else if (command.compare("print") == 0) //���� ���̺� ���
		{
			Print_table(flashmem, mapping_method, table_type);
			system("pause");
		}
		else if (command.compare("searchmode") == 0) //�� ������ Ž�� ��� ����
		{
			flashmem->switch_search_mode(flashmem, mapping_method);
		}
		else if (command.compare("ebqprint") == 0 || table_type == TABLE_TYPE::DYNAMIC) //Empty Block ť ���
		{
			flashmem->empty_block_queue->print();
		}
		else if (command.compare("sbqprint") == 0) //Spare Block ť ���
		{
			flashmem->spare_block_queue->print();
		}
		else if (command.compare("vbqprint") == 0) //Victim Block ť ���
		{
			flashmem->victim_block_queue->print();
		}
		else if (command.compare("trace") == 0) //Ʈ���̽� ����
		{
			trace(flashmem, mapping_method, table_type);
			system("pause");
		}
		else if (command.compare("clrglobalcnt") == 0) //�÷��� �޸��� ��ü Read, Write, Erase Ƚ�� �ʱ�ȭ
		{
			flashmem->v_flash_info.clear_trace_info();
		}
		else if (command.compare("lbninfo") == 0) //LBN�� ������ PBN meta ���� ���
		{
			ss >> LBN;

			if (LBN < 0)
			{
				std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
				system("pause");
				break;
			}
			
			print_block_meta_info(flashmem, true, LBN, mapping_method);
			system("pause");
		}
		else if (command.compare("pbninfo") == 0) //PBN meta ���� ���
		{
			ss >> PBN;
			
			if (PBN < 0)
			{
				std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
				system("pause");
				break;
			}
			
			print_block_meta_info(flashmem, false, PBN, mapping_method);
			system("pause");
		}
		else if (command.compare("info") == 0) //���� ���
		{
			flashmem->disp_flash_info(flashmem, mapping_method, table_type);
			system("pause");
		}
		else if (command.compare("exit") == 0) //����
		{
			if (flashmem != NULL)
			{
				flashmem->gc->RDY_terminate = true; //���� ��� ���� �˸�
				flashmem->gc->scheduler(flashmem, mapping_method, table_type);
			}
			else
				exit(1);
		}
		else if (command.compare("cleaner") == 0)
		{
			system("cleaner.cmd");
		}
		else
		{
			std::cout << "�߸��� ��ɾ� �Է�" << std::endl;
			system("pause");
			break;
		}
		break;
	}
}

void FlashMem::disp_flash_info(FlashMem*& flashmem, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //���� ������ �÷��� �޸��� ���� ���
{
	if (flashmem != NULL) //���� ������ �÷��� �޸��� ���� �����ֱ�
	{
		F_FLASH_INFO f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

		/***
			���������� �����ִ� ��� ���� ���� = ��ü byte���� �� - (��ϵ� ���� �� * ���� �� ����Ʈ ��)
			=> ����ڿ��� �������� �ʴ� �뷮�̹Ƿ�, Spare Block�� ���Խ�Ų��.

			�������� �����ִ� ��� ������ ���� ������ ������ ����� �Ұ����� ������ Spare Block�� �����ϴ� �� byte���� �����Ѵ�
		***/
	
		unsigned int physical_using_space = flashmem->v_flash_info.written_sector_count * SECTOR_INC_SPARE_BYTE; //���������� ��� ���� ����
		unsigned int physical_free_space = f_flash_info.storage_byte - physical_using_space; //���������� �����ִ� ��� ���� ����

		//�������� �����ִ� ��� ���� ���� = ��ü byte���� �� - (��ϵ� ���͵� �� ��ȿ ���� ���� * ���� �� ����Ʈ ��) - Spare Block�� �����ϴ� �� byte��
		unsigned int logical_using_space = (flashmem->v_flash_info.written_sector_count - flashmem->v_flash_info.invalid_sector_count) * SECTOR_INC_SPARE_BYTE;
		unsigned int logical_free_space = (f_flash_info.storage_byte - logical_using_space) - f_flash_info.spare_block_byte;
		
		float physical_used_percent = ((float)physical_using_space / (float)f_flash_info.storage_byte) * 100;
		float logical_used_percent = ((float)logical_using_space / ((float)f_flash_info.storage_byte - (float)f_flash_info.spare_block_byte)) * 100;
		
		if (mapping_method != MAPPING_METHOD::NONE)
		{
			std::cout << "���̺� Ÿ�� : ";
			switch (table_type)
			{
			case TABLE_TYPE::STATIC:
				std::cout << "Static Table" << std::endl;
				break;

			case TABLE_TYPE::DYNAMIC:
				std::cout << "Dynamic Table" << std::endl;
				break;

			default:
				goto WRONG_TABLE_TYPE_ERR;
			}
		}
		std::cout << "���� ������ �÷��� �޸��� �뷮 : " << f_flash_info.flashmem_size << "MB(" << f_flash_info.storage_byte << "bytes)" << std::endl;
		std::cout << "��ü ��� �� : " << f_flash_info.block_size << " [���� : 0~" << f_flash_info.block_size - 1 << "]" << std::endl;
		std::cout << "��ü ���� �� : " << f_flash_info.sector_size << " [���� : 0~" << f_flash_info.sector_size - 1 << "]" << std::endl;
		std::cout << "-----------------------------------------------------" << std::endl;
		/*** 
			���� ����� ��� �� �� ���, Overwrite�� �Ұ����ϹǷ�, ����(������)���� �Ǵ� ��� ���� Invalid ó���� �߻����� ����
			����, ���������� �����ִ� ���� ��� ���� ������ �������� �����ִ� ��� ���� ���� (����ڿ��� �������� �뷮)�� �׻� �����ϴ�.
		***/
		std::cout << "���������� �����ִ� ��� ���� ���� (Spare Block ����) : " << physical_free_space << "bytes" << std::endl;
		printf("��� �� %ubytes / ��ü %ubytes (%.1f%% used)\n", physical_using_space, f_flash_info.storage_byte, physical_used_percent);
		std::cout << "�������� �����ִ� ��� ���� ���� (Spare Block �� ����, ����ڿ��� �������� �뷮) : " << logical_free_space << "bytes" << std::endl;
		printf("��� �� %ubytes / ��ü %ubytes (%.1f%% used)\n", logical_using_space, f_flash_info.storage_byte - f_flash_info.spare_block_byte, logical_used_percent);
		std::cout << "-----------------------------------------------------" << std::endl;
		std::cout << "��ü Spare Area ���� ũ�� : " << f_flash_info.spare_area_byte << "bytes" << std::endl;
		std::cout << "Spare Area ���� ��ü ������ ���� ũ�� : " << f_flash_info.data_only_storage_byte << "bytes" << std::endl;
		std::cout << "-----------------------------------------------------" << std::endl;
		std::cout << "Current Flash read count : " << flashmem->v_flash_info.flash_read_count << std::endl;
		std::cout << "Current Flash write count : " << flashmem->v_flash_info.flash_write_count << std::endl;
		std::cout << "Current Flash erase count : " << flashmem->v_flash_info.flash_erase_count << std::endl;
		std::cout << "-----------------------------------------------------" << std::endl;
		
		if (mapping_method != MAPPING_METHOD::NONE) //���� ����� ����ϸ�
		{
			std::cout << "��ü Spare Block �� : " << f_flash_info.spare_block_size << "��" << std::endl;
			std::cout << "��ü Spare Block ũ�� : " << f_flash_info.spare_block_byte << "bytes" << std::endl;
			std::cout << "-----------------------------------------------------" << std::endl;
			flashmem->gc->print_invalid_ratio_threshold();
			std::cout << "-----------------------------------------------------" << std::endl;
			std::cout << "���� �� ������ Ž�� ��� : ";
			switch (flashmem->search_mode)
			{
			case SEARCH_MODE::SEQ_SEARCH:
				std::cout << "Sequential search" << std::endl;
				break;

			case SEARCH_MODE::BINARY_SEARCH:
				std::cout << "Binary search" << std::endl;
				break;
			}
		}
	}
	else
	{
		std::cout << "not initialized" << std::endl;
	}

	return;

WRONG_TABLE_TYPE_ERR: //�߸��� ���̺� Ÿ��
	fprintf(stderr, "ġ���� ���� : �߸��� ���̺� Ÿ�� (disp_flash_info)\n");
	system("pause");
	exit(1);
}

void FlashMem::switch_mapping_method(MAPPING_METHOD& mapping_method, TABLE_TYPE& table_type) //���� �÷��� �޸��� ���� ��� �� ���̺� Ÿ�� ����
{
	while (1)
	{
		int input_mapping_method = -1;
		int input_table_type = -1;

		system("cls");
		std::cout << "=============================================================================" << std::endl;
		std::cout << "!! ���� ��� �� ���̺� Ÿ�� ���� �� ���� �÷��� �޸𸮸� ����(init)�Ͽ��� ��" << std::endl;
		std::cout << "=============================================================================" << std::endl;
		std::cout << "0 : Do not use any Mapping Method" << std::endl;
		std::cout << "2 : Block Mapping Method (Static Table, Dynamic Table)" << std::endl;
		std::cout << "3 : Hybrid Mapping Method (log algorithm - 1:2 block level mapping)" << std::endl;
		std::cout << "=============================================================================" << std::endl;
		std::cout << ">>";
		std::cin >> input_mapping_method;

		switch (input_mapping_method)
		{
		case MAPPING_METHOD::NONE: //non-ftl
			break;

		case MAPPING_METHOD::BLOCK: //block mapping
			std::cin.clear(); //������Ʈ�� �ʱ�ȭ
			std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���

			std::cout << "=============================================================================" << std::endl;
			std::cout << "!! ���� ��� �� ���̺� Ÿ�� ���� �� ���� �÷��� �޸𸮸� ����(init)�Ͽ��� ��" << std::endl;
			std::cout << "=============================================================================" << std::endl;
			std::cout << "0 : Static Table" << std::endl;
			std::cout << "1 : Dynamic Table" << std::endl;
			std::cout << "=============================================================================" << std::endl;
			std::cout << ">>";
			std::cin >> input_table_type;

			if (input_table_type != TABLE_TYPE::STATIC && input_table_type != TABLE_TYPE::DYNAMIC)
			{
				std::cin.clear(); //������Ʈ�� �ʱ�ȭ
				std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���
				continue;
			}

			break;

		case MAPPING_METHOD::HYBRID_LOG: //hybrid mapping
			input_table_type = TABLE_TYPE::DYNAMIC; //Dynamic Table ����
			break;

		default:
			break;
		}

		std::cin.clear(); //������Ʈ�� �ʱ�ȭ
		std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���

		if(input_mapping_method != -1)
			mapping_method = (MAPPING_METHOD)input_mapping_method;
	
		if(input_table_type != -1)
			table_type = (TABLE_TYPE)input_table_type;

		break;
	}
}

void FlashMem::switch_search_mode(FlashMem*& flashmem, MAPPING_METHOD mapping_method) //���� �÷��� �޸��� �� ������ Ž�� �˰��� ����
{
	if (mapping_method == MAPPING_METHOD::BLOCK) //��� ������ ��� ���� Ž���� ����
	{
		std::cout << "������ ���� ������ ��� ���� ������ ���� �Ұ���" << std::endl;
		return;
	}

	while (1)
	{
		int input_search_mode = -1;

		system("cls");
		std::cout << "=============================================================================" << std::endl;
		std::cout << "0 : Sequential Search" << std::endl;
		std::cout << "1 : Binary Search" << std::endl;
		std::cout << "-----------------------------------------------------------------------------" << std::endl;
		std::cout << "���� �� ������ Ž�� ��� : ";
		switch (flashmem->search_mode)
		{
		case SEARCH_MODE::SEQ_SEARCH:
			std::cout << "Sequential search" << std::endl;
			break;

		case SEARCH_MODE::BINARY_SEARCH:
			std::cout << "Binary search" << std::endl;
			break;
		}
		std::cout << "-----------------------------------------------------------------------------" << std::endl;
		std::cout << ">>";

		std::cin >> input_search_mode;
		if (input_search_mode != SEARCH_MODE::SEQ_SEARCH && input_search_mode != SEARCH_MODE::BINARY_SEARCH)
		{
			std::cin.clear(); //������Ʈ�� �ʱ�ȭ
			std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���
			continue;
		}
		else
		{
			std::cin.clear(); //������Ʈ�� �ʱ�ȭ
			std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���
			flashmem->search_mode = (SEARCH_MODE)input_search_mode;
			break;
		}
	}
	return;
}

FIXED_FLASH_INFO FlashMem::get_f_flash_info() //�÷��� �޸��� ������ ���� ����
{
	return this->f_flash_info;
}