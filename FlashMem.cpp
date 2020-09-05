#include "FlashMem.h"

//��ɾ� ��� ���, �÷��� �޸��� ����, ���� ���̺� �ҷ����� �� ���丮�� �뷮�� ����ϴ� ���� �Լ� ����

void VICTIM_BLOCK_INFO::clear_all() //Victim Block ������ ���� ��� ���� ����ü �ʱ�ȭ
{
	this->is_logical = true;

	//���� �� ���� ������ �ʱ�ȭ
	this->victim_block_num = DYNAMIC_MAPPING_INIT_VALUE;
	this->victim_block_invalid_ratio = -1;
}

void VARIABLE_FLASH_INFO::clear_all() //��� �ʱ�ȭ
{
	/*** Variable Information ***/
	this->written_sector_count = 0;
	this->invalid_sector_count = 0;

	//for trace
	this->flash_write_count = 0;
	this->flash_erase_count = 0;
	this->flash_read_count = 0;
}

void VARIABLE_FLASH_INFO::clear_trace_info() //trace�� ���� ���� �ʱ�ȭ
{
	//for trace
	this->flash_write_count = 0;
	this->flash_erase_count = 0;
	this->flash_read_count = 0;
}

void BLOCK_TRACE_INFO::clear_all() //��� �� ���� trace�� ���� ���� �ʱ�ȭ
{
	this->block_write_count = 0;
	this->block_read_count = 0;
	this->block_erase_count = 0;
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

	/*** ���̺� �ʱ�ȭ : �Ҵ� ���̹Ƿ� �����͸� �ʱ�ȭ ***/
	this->block_level_mapping_table = NULL; //��� ���� ���̺�
	this->log_block_level_mapping_table = NULL; //1 : 2 ��� ���� ���̺� (Log Algorithm)
	this->offset_level_mapping_table = NULL; //������ ���� ���� ���̺�
	this->spare_block_table = NULL; //Spare ��� ���̺�

	/*** Variable Information ***/
	this->v_flash_info.clear_all();

	/*** Victim Block ������ ���� ��� ���� ����ü ***/
	this->victim_block_info.clear_all();
	this->victim_block_queue = NULL;

	this->gc = NULL;

	this->block_trace_info = NULL;
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

	/*** ���̺� �ʱ�ȭ : �Ҵ� ���̹Ƿ� �����͸� �ʱ�ȭ ***/
	this->block_level_mapping_table = NULL; //��� ���� ���̺�
	this->log_block_level_mapping_table = NULL; //1 : 2 ��� ���� ���̺� (Log Algorithm)
	this->offset_level_mapping_table = NULL; //������ ���� ���� ���̺�
	this->spare_block_table = NULL; //Spare ��� ���̺�

	/*** Variable Information ***/
	this->v_flash_info.clear_all();

	/*** Victim Block ������ ���� ��� ���� ����ü ***/
	this->victim_block_info.clear_all();
	this->victim_block_queue = NULL;

	/*** ������ �ݷ��� ��ü ���� ***/
	this->gc = new GarbageCollector();

	/*** ��� �� ���� ���� ***/
	this->block_trace_info = NULL;

#if BLOCK_TRACE_MODE == 1
	this->block_trace_info = new BLOCK_TRACE_INFO[this->f_flash_info.block_size]; //��ü ��� ���� ũ��� �Ҵ�
	for (unsigned int PBN = 0; PBN < this->f_flash_info.block_size; PBN++)
		block_trace_info[PBN].clear_all(); //�б�, ����, ����� Ƚ�� �ʱ�ȭ
#endif
}

FlashMem::~FlashMem()
{
	this->deallocate_table();
	delete this->gc;

#if BLOCK_TRACE_MODE == 1
	delete[] this->block_trace_info;
#endif
}

void FlashMem::bootloader(FlashMem** flashmem, int& mapping_method, int& table_type) //Reorganization process from initialized flash memory storage file
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

	if ((*flashmem) == NULL)
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
					unsigned short megabytes;
					fscanf(volume, "%hd\n", &megabytes);
					(*flashmem) = new FlashMem(megabytes); //�뷮 �����Ͽ� ����
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
		(*flashmem)->load_table(mapping_method);

		/*** 
			1) ������ ���丮�� ���Ϸκ��� ������ ���丮�� ������ ���� (��ϵ� ���� ��, ��ȿȭ�� ���� ��)
			2) ������ ���丮�� ������ ���� Victim Block ���� ���� ������ ��ȿ�� �Ӱ谪 ����
			----
			!! ������ ���丮�� ������ ���� �� Victim Block ť �� ������ ���� ��� ��� �� ������ �ʿ��� ������尡 ����� ũ�� ������
			������ ���丮�� ������ �����ϸ鼭 ���� ��ȿȭ�� ���� ���(valid_block == false)�� Victim Block ť�� �����Ѵ�.
		***/
		f_flash_info = (*flashmem)->get_f_flash_info();

		/*** ���� ����� ����� ��� GC�� ���� Victim Block ť ���� ***/
		switch (mapping_method)
		{
		case 0:
			break;

		default:
			(*flashmem)->victim_block_queue = new Victim_Queue(f_flash_info.block_size);
			break;
		}
		
		for (unsigned int PBN = 0; PBN < f_flash_info.block_size; PBN++)
		{
			printf("Reorganizing...(%.0f%%)\r", ((float)PBN / (float)(f_flash_info.block_size - 1)) * 100);
			block_meta_buffer_array = SPARE_reads(flashmem, PBN); //�ش� ����� ��� ����(������)�� ���� meta������ �о��

			if (block_meta_buffer_array[0]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] == false) //��ȿȭ�� ����̸�
			{
				/***
					update_victim_block_infoȣ�� �� meta������ �ߺ��Ͽ� �ٽ� �����Ƿ�, 
					�ش� �ҽ��� �Ϻκ��� ����Ѵ�
				***/
				(*flashmem)->victim_block_info.is_logical = false; //PBN
				(*flashmem)->victim_block_info.victim_block_num = PBN;
				(*flashmem)->victim_block_info.victim_block_invalid_ratio = 1.0;
			}

			if (update_v_flash_info_for_reorganization(flashmem, block_meta_buffer_array) != SUCCESS)
				goto WRONG_META_ERR;

			/*** Deallocate block_meta_buffer_array ***/
			if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
				goto MEM_LEAK_ERR;

			/*** ��ȿȭ�� ��� ���� �� Victim Block ť ����, �̿� ���� ť�� ���� �� �� ó�� ***/
			(*flashmem)->gc->scheduler(flashmem, mapping_method);
		}

		(*flashmem)->gc->RDY_v_flash_info_for_set_invalid_ratio_threshold = true; //��ȿ�� �Ӱ谪 ������ ���� ������ ���丮�� ���� ���� �Ϸ� �˸�

		/*** ���� �Ϸ�� ������ ���丮�� ������ ���� GC�� ���� ������ ��ȿ�� �Ӱ谪 ���� ***/
		(*flashmem)->gc->scheduler(flashmem, mapping_method);
		(*flashmem)->gc->print_invalid_ratio_threshold();

		printf("Reorganizing Process Success\n");
		system("pause");
	}

	if (volume != NULL)
		fclose(volume);
	return;

WRONG_META_ERR: //�߸��� meta���� ����
	fprintf(stderr, "���� : �߸��� meta ���� (bootloader)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "���� : meta ������ ���� �޸� ���� �߻� (bootloader)\n");
	system("pause");
	exit(1);
}

void FlashMem::load_table(int mapping_method) //���ι�Ŀ� ���� ���� ���̺� �ε�
{
	/***
		< ���̺� load ���� >
	
		1) Block level(normal or log block) table
		2) Spare Block table
		3) Offset level table
	***/

	FILE* table = NULL;

	unsigned int* spare_block_table_buffer = NULL; //Spare_Block_Table�� seq_record()�� ���� ���̺� �� ���� �Ҵ� ���� ����

	//����� ���� ���̺�κ��� �Ҵ� ����

	if ((table = fopen("table.bin", "rb")) == NULL) //�б� + �������� ���
	{
		fprintf(stderr, "table.bin ������ �б���� �� �� �����ϴ�. (load_table)");
		return;
	}

	switch (mapping_method) //���� ��Ŀ� ���� ĳ���� �����Ҵ� �� �ҷ�����
	{
	case 1:
		return;

	case 2: //��� ����
		if (this->block_level_mapping_table == NULL && this->spare_block_table == NULL)
		{
			//��� ���� ���� ���̺�, Spare ��� ���̺�
			this->block_level_mapping_table = new unsigned int[this->f_flash_info.block_size - this->f_flash_info.spare_block_size];
			fread(this->block_level_mapping_table, sizeof(unsigned int), this->f_flash_info.block_size - this->f_flash_info.spare_block_size, table);

			spare_block_table_buffer = new unsigned int[f_flash_info.spare_block_size];
			this->spare_block_table = new Spare_Block_Table(f_flash_info.spare_block_size);

			fread(spare_block_table_buffer, sizeof(unsigned int), this->f_flash_info.spare_block_size, table);
			/*** ���ۿ� �� ���� ���� ���� seq_write�� ���� ���� �Ҵ� ***/
			for (unsigned int spare_block_table_buffer_index = 0; spare_block_table_buffer_index < this->f_flash_info.spare_block_size; spare_block_table_buffer_index++)
			{
				this->spare_block_table->seq_write(spare_block_table_buffer[spare_block_table_buffer_index]);
			}
		}
		else
			goto LOAD_FAIL;
		break;

	case 3: //���̺긮�� ���� (Log algorithm - 1:2 Block level mapping with Dynamic Table)
		if (this->log_block_level_mapping_table == NULL && this->spare_block_table == NULL && this->offset_level_mapping_table == NULL)
		{
			//��� ���� 1:2 ���� ���̺�, Spare ��� ���̺�, ������ ���� ���̺�
			this->log_block_level_mapping_table = new unsigned int* [this->f_flash_info.block_size - this->f_flash_info.spare_block_size]; //row : ��ü PBN�� ��

			for (unsigned int i = 0; i < this->f_flash_info.block_size - this->f_flash_info.spare_block_size; i++)
			{
				log_block_level_mapping_table[i] = new unsigned int[2]; //col : �� ������ ���� PBN1, PBN2�� ��Ÿ��
				//�� ���� ������ �Ҵ��߱� ������ �� ���� ��ü �迭�� ���� �� ����. �� ������ ����
				fread(this->log_block_level_mapping_table[i], sizeof(unsigned int), 2, table);
			}

			spare_block_table_buffer = new unsigned int[f_flash_info.spare_block_size];
			this->spare_block_table = new Spare_Block_Table(f_flash_info.spare_block_size);
			fread(spare_block_table_buffer, sizeof(unsigned int), this->f_flash_info.spare_block_size, table);
			/*** ���ۿ� �� ���� ���� ���� seq_write�� ���� ���� �Ҵ� ***/
			for (unsigned int spare_block_table_buffer_index = 0; spare_block_table_buffer_index < this->f_flash_info.spare_block_size; spare_block_table_buffer_index++)
			{
				this->spare_block_table->seq_write(spare_block_table_buffer[spare_block_table_buffer_index]);
			}

			this->offset_level_mapping_table = new __int8[this->f_flash_info.block_size * BLOCK_PER_SECTOR];
			fread(this->offset_level_mapping_table, sizeof(__int8), this->f_flash_info.block_size * BLOCK_PER_SECTOR, table);
		}
		else
			goto LOAD_FAIL;
		break;

	default:
		return;
	}

	if (spare_block_table_buffer != NULL)
		delete[] spare_block_table_buffer;

	fclose(table);
	return;

LOAD_FAIL:
	fprintf(stderr, "���� : ���̺� �ҷ����� ����\n");
	system("pause");
	exit(1);
}
void FlashMem::save_table(int mapping_method) //���ι�Ŀ� ���� ���� ���̺� ����
{
	/***
		< ���̺� save ���� >
		
		1) Block level(normal or log block) table
		2) Spare Block table
		3) Offset level table
	***/

	FILE* table = NULL;

	if ((table = fopen("table.bin", "wb")) == NULL) //���� + �������� ���
	{
		fprintf(stderr, "table.bin ������ ������� �� �� �����ϴ�. (save_table)");
		return;
	}

	switch (mapping_method) //���� ��Ŀ� ���� ����
	{
	case 2: //��� ����
		//��� ���� ���� ���̺�, Spare ��� ���̺�
		if (this->block_level_mapping_table != NULL && this->spare_block_table != NULL)
		{
			fwrite(this->block_level_mapping_table, sizeof(unsigned int), this->f_flash_info.block_size - this->f_flash_info.spare_block_size, table);
			fwrite(this->spare_block_table->table_array, sizeof(unsigned int), this->f_flash_info.spare_block_size, table);
		}
		else
			goto SAVE_FAIL;
		break;

	case 3: //���̺긮�� ���� (Log algorithm - 1:2 Block level mapping with Dynamic Table)
		//��� ���� 1:2 ���� ���̺�, Spare ��� ���̺�, ������ ���� ���̺�
		if (this->log_block_level_mapping_table != NULL && this->spare_block_table != NULL && this->offset_level_mapping_table != NULL)
		{
			//�� ���� ������ �Ҵ��߱� ������ �� ���� ��ü �迭�� �� �� ����. �� ������ ����
			for (unsigned int i = 0; i < this->f_flash_info.block_size - this->f_flash_info.spare_block_size; i++)
			{
				fwrite(this->log_block_level_mapping_table[i], sizeof(unsigned int), 2, table);
			}
			fwrite(this->spare_block_table->table_array, sizeof(unsigned int), this->f_flash_info.spare_block_size, table); //Spare block ���̺� ���
			fwrite(this->offset_level_mapping_table, sizeof(__int8), this->f_flash_info.block_size * BLOCK_PER_SECTOR, table); //������ ���� ���� ���̺� ���
		}
		else
			goto SAVE_FAIL;
		break;

	default:
		return;
	}

	fclose(table);
	return;

SAVE_FAIL:
	fprintf(stderr, "���� : ���̺� ���� ����\n");
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

	//Spare ��� ���̺�
	if (this->spare_block_table != NULL)
	{
		delete this->spare_block_table;
		this->spare_block_table = NULL;
	}
}

void FlashMem::disp_command(int mapping_method, int table_type) //Ŀ�ǵ� ��ɾ� ���
{
	switch (mapping_method)
	{
	case 0:
		system("cls");
		std::cout << "========================================================" << std::endl;
		std::cout << "	�÷��� �޸� �ùķ����� - Non-FTL" << std::endl;
		std::cout << "========================================================" << std::endl;
		std::cout << " init x �Ǵ� i x - x MB ��ŭ �÷��� �޸� ���� " << std::endl;
		std::cout << " read PSN �Ǵ� r PSN - ���� ������ ������ �б�" << std::endl;
		std::cout << " write PSN data �Ǵ� w PSN data - ���� ������ data ���" << std::endl;
		std::cout << " erase PBN �Ǵ� e PBN - ���� ����� ������ ����" << std::endl;
		std::cout << "--------------------------------------------------------" << std::endl;
		std::cout << " change - ���� ��� ����" << std::endl;
		std::cout << "--------------------------------------------------------" << std::endl;
		std::cout << " pbninfo PBN - �ش� ���� ����� ��� ������ meta���� ���" << std::endl;
		std::cout << "--------------------------------------------------------" << std::endl;
		std::cout << " info - �÷��� �޸� ���� ���" << std::endl;
		std::cout << " exit - ����" << std::endl;
		std::cout << "========================================================" << std::endl;
		break;

	case 1:
		break;

	case 2:
		system("cls");
		std::cout << "====================================================================" << std::endl;
		std::cout << "	�÷��� �޸� �ùķ����� - Block Mapping ";
		if(table_type == 0) std::cout << "(Static Table)" << std::endl;
		else std::cout << "(Dynamic Table)" << std::endl;
		std::cout << "====================================================================" << std::endl;
		std::cout << " init x �Ǵ� i x - x MB ��ŭ �÷��� �޸� ���� " << std::endl;
		std::cout << " read LSN �Ǵ� r LSN - �� ������ ������ �б�" << std::endl;
		std::cout << " write LSN data �Ǵ� w LSN data - �� ������ data ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " change - ���� ��� ����" << std::endl;
		std::cout << " print - ���� ���̺� ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " trace - trace���Ϸκ��� ���� ���� ����  " << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		//std::cout << " gc - ������ ������ �÷��� �ǽ�  " << std::endl;
		std::cout << " vqprint - Victim Block ť ���  " << std::endl;
		std::cout << " lbninfo LBN - �ش� LBN�� ������ ���� ����� ��� ������ meta���� ���" << std::endl;
		std::cout << " pbninfo PBN - �ش� ���� ����� ��� ������ meta���� ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " info - �÷��� �޸� ���� ���" << std::endl;
		std::cout << " exit - ����" << std::endl; //Victim Block ť�� ��� ���� ���� ���� ����(GC�� �۾����� �ƴ� ��)
		std::cout << "====================================================================" << std::endl;
		break;

	case 3:
		system("cls");
		std::cout << "====================================================================" << std::endl;
		std::cout << "	�÷��� �޸� �ùķ����� - Hybrid Mapping ";
		if (table_type == 0) std::cout << "(Static Table)" << std::endl;
		else std::cout << "(Dynamic Table)" << std::endl;
		std::cout << "====================================================================" << std::endl;
		std::cout << " init x �Ǵ� i x - x MB ��ŭ �÷��� �޸� ���� " << std::endl;
		std::cout << " read LSN �Ǵ� r LSN - �� ������ ������ �б�" << std::endl;
		std::cout << " write LSN data �Ǵ� w LSN data - �� ������ data ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " change - ���� ��� ����" << std::endl;
		std::cout << " print - ���� ���̺� ���" << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " trace - trace���Ϸκ��� ���� ���� ����  " << std::endl;
		//std::cout << " gc - ������ ������ �÷��� �ǽ�  " << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl;
		std::cout << " vqprint - Victim Block ť ���  " << std::endl;
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

void FlashMem::input_command(FlashMem** flashmem, int& mapping_method, int& table_type) //Ŀ�ǵ� ��ɾ� �Է�
{
	std::string user_input; //����ڷκ��� ��ɾ �� �ٷ� �Է¹޴� ����
	
	/*** user_input���κ��� �и��ϴ� ���� ***/
	std::string command; //��ɾ�
	char data = NULL; //����ϰ��� �ϴ� ������
	__int64 megabytes = -1; //�����ϰ��� �ϴ� �÷��� �޸� �뷮
	__int64 LSN = -1; //�� ���� ��ȣ
	__int64 LBN = -1; //�� ��� ��ȣ
	__int64 PBN = -1; //���� ��� ��ȣ
	__int64 PSN = -1; //���� ���� ��ȣ
	//�� ���� ó���� ���� __int64������ �޴´�

	char data_output = NULL; //���� ���ͷκ��� �о���� ������
	
	std::cout << "��ɾ� >> ";
	std::getline(std::cin, user_input); //�� �ٷ� �Է¹ޱ�
	std::stringstream ss(user_input); //�и�
	ss >> command;

	switch (mapping_method)
	{
	case 0: //���� ������� ����
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
			if(Flash_read(flashmem, NULL, PSN, data_output) != FAIL)
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
			Flash_write(flashmem, NULL, PSN, data);
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
			(*flashmem)->switch_mapping_method(mapping_method, table_type);
			if(*flashmem != NULL)
				(*flashmem)->deallocate_table(); //���� ���̺� ����
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
			(*flashmem)->disp_flash_info(flashmem, mapping_method, table_type);
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
			(*flashmem)->switch_mapping_method(mapping_method, table_type);
			
			if(*flashmem != NULL) //�������Ͽ� �����Ǿ��ִ��� Ȯ��
				(*flashmem)->deallocate_table(); //���� ���̺� ����
		}
		else if (command.compare("print") == 0) //���� ���̺� ���
		{
			Print_table(flashmem, mapping_method, table_type);
			system("pause");
		}
		
		else if (command.compare("vqprint") == 0) //Victim Block ť ���
		{
			(*flashmem)->victim_block_queue->print();
		}
		else if (command.compare("trace") == 0) //Ʈ���̽� ����
		{
			trace(flashmem, mapping_method, table_type);
			system("pause");
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
			(*flashmem)->disp_flash_info(flashmem, mapping_method, table_type);
			system("pause");
		}
		else if (command.compare("exit") == 0) //����
		{
			if (*flashmem != NULL)
			{
				(*flashmem)->gc->RDY_terminate = true; //���� ��� ���� �˸�
				(*flashmem)->gc->scheduler(flashmem, mapping_method);
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

void FlashMem::disp_flash_info(FlashMem** flashmem, int mapping_method, int table_type) //���� ������ �÷��� �޸��� ���� ���
{
	if (*flashmem != NULL) //���� ������ �÷��� �޸��� ���� �����ֱ�
	{
		F_FLASH_INFO f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

		/***
			���������� �����ִ� ��� ���� ���� = ��ü byte���� �� - (��ϵ� ���� �� * ���� �� ����Ʈ ��)
			=> ����ڿ��� �������� �ʴ� �뷮�̹Ƿ�, Spare Block�� ���Խ�Ų��.

			�������� �����ִ� ��� ������ ���� ������ ������ ����� �Ұ����� ������ Spare Block�� �����ϴ� �� byte���� �����Ѵ�
		***/
	
		unsigned int physical_using_space = (*flashmem)->v_flash_info.written_sector_count * SECTOR_INC_SPARE_BYTE; //���������� ��� ���� ����
		unsigned int physical_free_space = f_flash_info.storage_byte - physical_using_space; //���������� �����ִ� ��� ���� ����

		//�������� �����ִ� ��� ���� ���� = ��ü byte���� �� - (��ϵ� ���͵� �� ��ȿ ���� ���� * ���� �� ����Ʈ ��) - Spare Block�� �����ϴ� �� byte��
		unsigned int logical_using_space = ((*flashmem)->v_flash_info.written_sector_count - (*flashmem)->v_flash_info.invalid_sector_count) * SECTOR_INC_SPARE_BYTE;
		unsigned int logical_free_space = (f_flash_info.storage_byte - logical_using_space) - f_flash_info.spare_block_byte;
		
		float physical_used_percent = ((float)physical_using_space / (float)f_flash_info.storage_byte) * 100;
		float logical_used_percent = ((float)logical_using_space / ((float)f_flash_info.storage_byte - (float)f_flash_info.spare_block_byte)) * 100;
		
		if (mapping_method != 0)
		{
			std::cout << "���̺� Ÿ�� : ";
			switch (table_type)
			{
			case 0:
				std::cout << "Static Table" << std::endl;
				break;

			case 1:
				std::cout << "Dynamic Table" << std::endl;
				break;
			default:
				break;
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
		std::cout << "�������� �����ִ� ��� ���� ���� (����ڿ��� �������� �뷮) : " << logical_free_space << "bytes" << std::endl;
		printf("��� �� %ubytes / ��ü %ubytes (%.1f%% used)\n", logical_using_space, f_flash_info.storage_byte - f_flash_info.spare_block_byte, logical_used_percent);
		std::cout << "-----------------------------------------------------" << std::endl;
		std::cout << "��ü Spare Area ���� ũ�� : " << f_flash_info.spare_area_byte << "bytes" << std::endl;
		std::cout << "Spare Area ���� ��ü ������ ���� ũ�� : " << f_flash_info.data_only_storage_byte << "bytes" << std::endl;
		std::cout << "-----------------------------------------------------" << std::endl;
		std::cout << "Current Flash read count : " << (*flashmem)->v_flash_info.flash_read_count << std::endl;
		std::cout << "Current Flash write count : " << (*flashmem)->v_flash_info.flash_write_count << std::endl;
		std::cout << "Current Flash erase count : " << (*flashmem)->v_flash_info.flash_erase_count << std::endl;
		std::cout << "-----------------------------------------------------" << std::endl;
		if (mapping_method != 0) //���� ����� ����ϸ�
		{
			std::cout << "��ü Spare Block �� : " << f_flash_info.spare_block_size << "��" << std::endl;
			std::cout << "��ü Spare Block ũ�� : " << f_flash_info.spare_block_byte << "bytes" << std::endl;
			std::cout << "-----------------------------------------------------" << std::endl;
			(*flashmem)->gc->print_invalid_ratio_threshold();
		}
	}
	else
	{
		std::cout << "not initialized" << std::endl;
	}

	return;
}

void FlashMem::switch_mapping_method(int& mapping_method, int& table_type) //���� �÷��� �޸��� ���� ��� �� ���̺� Ÿ�� ����
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
		case 0: //non-ftl
			break;

		case 2: //block mapping
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

			if (input_table_type != 0 && input_table_type != 1)
			{
				std::cin.clear(); //������Ʈ�� �ʱ�ȭ
				std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���
				continue;
			}

			break;

		case 3: //hybrid mapping
			input_table_type = 1; //Dynamic Table
			break;

		default:
			break;
		}

		std::cin.clear(); //������Ʈ�� �ʱ�ȭ
		std::cin.ignore(INT_MAX, '\n'); //�Է¹��ۺ���

		if(input_mapping_method != -1)
			mapping_method = input_mapping_method;
	
		if(input_table_type != -1)
			table_type = input_table_type;

		break;
	}
}

FIXED_FLASH_INFO FlashMem::get_f_flash_info() //�÷��� �޸��� ������ ���� ����
{
	return this->f_flash_info;
}