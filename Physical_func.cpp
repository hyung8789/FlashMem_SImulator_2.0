#include "Build_Options.h"

// init, read, write, erase �Լ� ����
// �÷��� �޸𸮿� ���� ���������� �����Ͽ� �۾�

int init(FlashMem*& flashmem, unsigned short megabytes, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //megabytes ũ���� �÷��� �޸𸮸� ����
{
	FILE* storage = NULL, //�÷��� �޸� ���丮�� ���� ������
		* volume = NULL;  //������ �÷��� �޸��� ���� (MB������ ũ��, ���� ���, ���̺� Ÿ��)�� �����ϱ� ���� ���� ������

	//for block mapping
	unsigned int* block_level_mapping_table = NULL; //��� ���� ���� ���̺�
	//for hybrid mapping
	unsigned int** log_block_level_mapping_table = NULL;  //1 : 2 ��� ���� ���� ���̺�(index : LBN, row : ��ü PBN�� ��, col : PBN1,PBN2)
	__int8* offset_level_mapping_table = NULL; //������ ����(0~31) ���� ���̺�

	unsigned char* data_inc_spare_array = NULL; //������ ���� + Spare Area�� char �迭
	unsigned char* spare_block_array = NULL; //������ ���� + Spare Area�� Spare Block�� ���� char �迭(Spare Area�� ���� �ʱⰪ ����)

	unsigned int init_next_pos = 0; //����� ���� ���� �������� ���� ��ġ
	unsigned int spare_block_num = DYNAMIC_MAPPING_INIT_VALUE; //Spare Block ��⿭�� �Ҵ� ���� Spare Block ��ȣ
	bool write_spare_block_proc = false;

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	//���� �÷��� �޸� ���� �� �� ����
	if (flashmem != NULL)
	{
		delete flashmem;
		flashmem = NULL;
	}
	remove("rr_read_index.txt"); //���� Spare Block Queue�� read_index ����

	flashmem = new FlashMem(megabytes); //���� �Ҵ�

	//Spare Block�� �����ϴ� ���ϴ� �����ؾ� �ϴ� ��ü ����(���) ���� ����
	//���͸��� Spare Area�� ����(512+16byte)�Ͽ�, Spare Area�� ������� ����(512byte) ���� ��(sector_size) ��ŭ ������ ��
	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (((storage = fopen("storage.bin", "wb")) == NULL) || ((volume = fopen("volume.txt", "wt")) == NULL)) //���丮�� ����, ���丮�� ���� ����
		goto NULL_FILE_PTR_ERR;

	/*** ���� ���̺�, Empty Block Queue, Spare Block Queue ���� �� �ʱ�ȭ ***/
	spare_block_num = f_flash_info.block_size - 1; //��ü ����� �� �ڿ������� ���������� �ʱ� Spare Block �Ҵ��� ���� ��ü ��� ��-1 

	switch (mapping_method) 
	{
	default:
		break;

	case MAPPING_METHOD::BLOCK: //��� ����
		block_level_mapping_table = new unsigned int[f_flash_info.block_size - f_flash_info.spare_block_size]; //Spare ��� ���� ������ ��ŭ�� ���� ���̺� ����
		flashmem->spare_block_queue = new Spare_Block_Queue(f_flash_info.spare_block_size + 1);
		
		if(table_type == TABLE_TYPE::DYNAMIC) //Static Table�� ��� ���� �߻� �� �� ��� �Ҵ��� �ʿ� �����Ƿ�, Empty Block Queue�� ������� �ʴ´�.
			flashmem->empty_block_queue = new Empty_Block_Queue((f_flash_info.block_size - f_flash_info.spare_block_size) + 1);

		//Spare ����� ��ü ����� �� �ڿ������� ���������� �Ҵ�
		for (unsigned int i = 0; i < f_flash_info.spare_block_size; i++)
		{
			if (flashmem->spare_block_queue->enqueue(spare_block_num--) == FAIL)
			{
				fprintf(stderr, "ġ���� ���� : Spare Block Queue �ʱ� �Ҵ� ����\n");
				system("pause");
				exit(EXIT_FAILURE);
			}
		}

		//table type�� ���� ���̺� �ʱ�ȭ, Static Table�� ��� Empty Block Queue�� ������� ����
		if (table_type == TABLE_TYPE::STATIC)
		{
			for (unsigned int table_index = 0; table_index < f_flash_info.block_size - f_flash_info.spare_block_size; table_index++)
			{
				block_level_mapping_table[table_index] = table_index;
			}
		}
		else //Dynamic table
		{
			for (unsigned int table_index = 0; table_index < f_flash_info.block_size - f_flash_info.spare_block_size; table_index++)
			{
				block_level_mapping_table[table_index] = DYNAMIC_MAPPING_INIT_VALUE;
				flashmem->empty_block_queue->enqueue(table_index); //����ִ� PBN�� ���� Empty Block Queue�� �Ҵ�
			}
		}
		break;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		log_block_level_mapping_table = new unsigned int*[f_flash_info.block_size - f_flash_info.spare_block_size]; //row : ��ü PBN�� ��
		offset_level_mapping_table = new __int8[f_flash_info.block_size * BLOCK_PER_SECTOR]; //������ ���� ���̺�(Spare Block ����)
		flashmem->spare_block_queue = new Spare_Block_Queue(f_flash_info.spare_block_size + 1);
		flashmem->empty_block_queue = new Empty_Block_Queue((f_flash_info.block_size - f_flash_info.spare_block_size) + 1);

		//Spare ����� ��ü ����� �� �ڿ������� ���������� �Ҵ�
		for (unsigned int i = 0; i < f_flash_info.spare_block_size; i++)
		{
			if (flashmem->spare_block_queue->enqueue(spare_block_num--) == FAIL)
			{
				fprintf(stderr, "ġ���� ���� : Spare Block Queue �ʱ� �Ҵ� ����\n");
				system("pause");
				exit(EXIT_FAILURE);
			}
		}

		for (unsigned int table_index = 0; table_index < f_flash_info.block_size - f_flash_info.spare_block_size; table_index++) //row1:PBN1
		{
			log_block_level_mapping_table[table_index] = new unsigned int[2]; //col : �� ������ ���� PBN1, PBN2�� ��Ÿ��
			log_block_level_mapping_table[table_index][0] = DYNAMIC_MAPPING_INIT_VALUE; //PBN1�� �ش��ϴ� ��ġ
			log_block_level_mapping_table[table_index][1] = DYNAMIC_MAPPING_INIT_VALUE; //PBN2�� �ش��ϴ� ��ġ
			flashmem->empty_block_queue->enqueue(table_index); //����ִ� PBN�� ���� Empty Block Queue�� �Ҵ�
		}

		for (unsigned int table_index = 0; table_index < f_flash_info.block_size * BLOCK_PER_SECTOR; table_index++)
		{
			offset_level_mapping_table[table_index] = OFFSET_MAPPING_INIT_VALUE;
		}
	
		break;
	}

	/*** ���̺� �޸𸮿� ĳ�� �� ���� ***/
	switch (mapping_method)
	{
	default:
		break;

	case MAPPING_METHOD::BLOCK: //��� ���� ���
		//���̺� ����
		flashmem->block_level_mapping_table = block_level_mapping_table;
		break;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		//���̺� ����
		flashmem->log_block_level_mapping_table = log_block_level_mapping_table;
		flashmem->offset_level_mapping_table = offset_level_mapping_table;
		break;
	}
	flashmem->save_table(mapping_method);

	/*** ���� ����� ����� ��� GC�� ���� Victim Block ť ���� ***/
	switch (mapping_method)
	{
	case MAPPING_METHOD::NONE:
		break;

	default:
		flashmem->victim_block_queue = new Victim_Block_Queue(round(f_flash_info.block_size * VICTIM_BLOCK_QUEUE_RATIO));
		break;
	}

	/*** Spare Area�� ������ 1���� ũ���� data_inc_spare_array ���� ***/
	data_inc_spare_array = new unsigned char[SECTOR_INC_SPARE_BYTE]; //Spare Area�� ������ ����(528����Ʈ) ũ���� �迭
	memset(data_inc_spare_array, NULL, SECTOR_INC_SPARE_BYTE); //NULL������ ��� �ʱ�ȭ (����Ʈ ����)
	for (int byte_unit = SECTOR_PER_BYTE; byte_unit < SECTOR_INC_SPARE_BYTE - 1; byte_unit++) //���� ��(0~527)�� 512 ~ 527 ���� Spare Area�� ���� �Ҵ�
	{
		data_inc_spare_array[byte_unit] = SPARE_INIT_VALUE; //0xff(16) = 11111111(2) = 255(10) �� �ʱ�ȭ
	}

	/*** 
		Spare Block�� ���� 1���� ũ���� spare_block_array ���� 
		0 ~ 511 : Data area
		512 ~ 527 : Spare area
	***/
	if (mapping_method != MAPPING_METHOD::NONE) //���� ��� ��� ��
	{
		spare_block_array = new unsigned char[SECTOR_INC_SPARE_BYTE];
		memset(spare_block_array, NULL, SECTOR_INC_SPARE_BYTE);
		spare_block_array[SECTOR_PER_BYTE] = (0x7f); //0x7f(16) = 01111111(2) = 127(10) �� �ʱ�ȭ

		for (int byte_unit = SECTOR_PER_BYTE + 1; byte_unit < SECTOR_INC_SPARE_BYTE; byte_unit++) //���� ��(0~527)�� 513 ~ 527 ���� Spare Area�� ���� �Ҵ�
		{
			spare_block_array[byte_unit] = SPARE_INIT_VALUE; //0xff(16) = 11111111(2) = 255(10)�� �ʱ�ȭ
		}
	}

	/*** �÷��� �޸� ���丮�� ���� ���� ***/
	init_next_pos = ftell(storage);
	write_spare_block_proc = false;

	if (mapping_method == MAPPING_METHOD::NONE) //non-FTL
	{
		while (1) //�Է¹��� MB��ŭ ���Ͽ� ���
		{

			fwrite(data_inc_spare_array, sizeof(unsigned char), SECTOR_INC_SPARE_BYTE, storage); //������ ���� ���� ���
			init_next_pos += SECTOR_INC_SPARE_BYTE;

			printf("%ubytes / %ubytes (%.1f%%)\r", init_next_pos, f_flash_info.storage_byte, ((float)init_next_pos / (float)(f_flash_info.storage_byte)) * 100);
			if (init_next_pos >= f_flash_info.storage_byte) break; //������ ����� ��ġ�� Spare Area�� ������ ��������� �뷮�� ���� ��� ����
		}
	}
	else //Spare Block �Ҵ�
	{
		while (1) //�Է¹��� MB��ŭ ���Ͽ� ���
		{
			if (!(write_spare_block_proc))
			{
				fwrite(data_inc_spare_array, sizeof(unsigned char), SECTOR_INC_SPARE_BYTE, storage); //������ ���� ���� ���
				init_next_pos += SECTOR_INC_SPARE_BYTE;
			}
			else
			{
				//ù��° �����¿� ���ؼ��� Spare Block ������ ������ spare_block_array�� ���
				fwrite(spare_block_array, sizeof(unsigned char), SECTOR_INC_SPARE_BYTE, storage); //������ ���� ���� (Spare block) ���
				for (__int8 offset_index = 1; offset_index < BLOCK_PER_SECTOR; offset_index++)
				{
					fwrite(data_inc_spare_array, sizeof(unsigned char), SECTOR_INC_SPARE_BYTE, storage); //������ ���� ���� (Spare block) ���
				}
				init_next_pos += (SECTOR_INC_SPARE_BYTE * BLOCK_PER_SECTOR);
			}
			
			if (init_next_pos >= f_flash_info.storage_byte - f_flash_info.spare_block_byte) //������ ����� ��ġ�� �ʱ� Spare Block ��ġ�� ���
				write_spare_block_proc = true; 

			printf("%ubytes / %ubytes (%.1f%%)\r", init_next_pos, f_flash_info.storage_byte, ((float)init_next_pos / (float)f_flash_info.storage_byte) * 100);
			if (init_next_pos >= f_flash_info.storage_byte) 
				break; //������ ����� ��ġ�� Spare Area�� ������ ��������� �뷮�� ���� ��� ����
		}
	}

	/*** ���丮�� ���� ���� ���� ���� ���� ***/
	delete[] data_inc_spare_array;
	delete[] spare_block_array;

	/*** �÷��� �޸� �뷮 �� ���� ��� ��� ***/
	fprintf(volume, "%hd\n", f_flash_info.flashmem_size); //������ �÷��� �޸��� MB ũ��
	fprintf(volume, "%d\n", mapping_method); //���� ���
	fprintf(volume, "%d", table_type); //���̺� Ÿ��

	printf("\n%u megabytes flash memory\n", f_flash_info.flashmem_size);

	fclose(storage);
	fclose(volume);

	flashmem->v_flash_info.flash_state = FLASH_STATE::IDLE;

	return SUCCESS;

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : storage.bin Ȥ�� volume.txt ������ ������� �� �� �����ϴ�. (init)\n");
	system("pause");
	exit(EXIT_FAILURE);
}

int Flash_read(FlashMem*& flashmem, class META_DATA*& dst_meta_buffer, unsigned int PSN, char& dst_data) //���� ���Ϳ� �����͸� �о��
{
	FILE* storage = NULL;
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����, FTL �˰����� ���� dst_buffer�� ����

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	unsigned int read_pos = 0; //�а��� �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����
	char read_buffer = NULL; //�о���� ������

	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (PSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	if ((storage = fopen("storage.bin", "rb")) == NULL) //�б� + �������� ���
		goto NULL_FILE_PTR_ERR;

	read_pos = SECTOR_INC_SPARE_BYTE * PSN; //�а��� �ϴ� ���� ����(������)�� ��ġ
	spare_pos = read_pos + SECTOR_PER_BYTE; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)

	if (dst_meta_buffer == NULL) //���� �������� meta���� ��û �� meta���� ����
	{
		fseek(storage, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�
		SPARE_read(flashmem, storage, meta_buffer);

		//���� ���� �������� ��ġ�� �а��� �ϴ� ���� ����(������)�� ���� ����(������)�� ���� ��ġ
		fseek(storage, -SECTOR_INC_SPARE_BYTE, SEEK_CUR); //�а��� �ϴ� ���� ����(������)�� ��ġ�� �ٽ� �̵�
		fread(&read_buffer, sizeof(char), 1, storage); //�ش� ���� ����(������)�� ��ϵ� �� �б�

		dst_meta_buffer = meta_buffer;
	}
	else //���� �������� �̹� Spare �ǵ� �Լ��� ���� �ش� ������ meta������ �ǵ� �Ͽ��� ���, Ȥ�� Non-FTL�� ���
	{
		fseek(storage, read_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� ��ġ�� �̵�
		fread(&read_buffer, sizeof(char), 1, storage); //�ش� ���� ����(������)�� ��ϵ� �� �б�

		/*** trace���� ���� ��� ***/
		flashmem->v_flash_info.flash_read_count++; //Global �÷��� �޸� �б� ī��Ʈ ����

#ifdef PAGE_TRACE_MODE //Trace for Per Sector(Page) Wear-leveling
		flashmem->page_trace_info[PSN].read_count++; //�ش� ����(������)�� �б� ī��Ʈ ����
#endif

#ifdef BLOCK_TRACE_MODE //Trace for Per Block Wear-leveling
		flashmem->block_trace_info[PSN / BLOCK_PER_SECTOR].read_count++; //�ش� ����� �б� ī��Ʈ ����
#endif

	}

	//�о���� ������ ���� ������ ����, ������ NULL ����
	dst_data = read_buffer;

	fclose(storage);

	if (read_buffer != NULL)
		return SUCCESS;
	else 
		return COMPLETE;

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : storage.bin ������ �б���� �� �� �����ϴ�. (Flash_read)\n");
	system("pause");
	exit(EXIT_FAILURE);
}

int Flash_write(FlashMem*& flashmem, class META_DATA*& src_meta_buffer, unsigned int PSN, const char src_data) //���� ���Ϳ� �����͸� ���
{
	FILE* storage = NULL;
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	unsigned int write_pos = 0; //������ �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����
	
	/*** �̹� �Էµ� ��ġ�� ������ �Է� �õ��� overwrite ���� �߻� ***/

	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (PSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	if ((storage = fopen("storage.bin", "rb+")) == NULL) //�а� ���� ��� + �������� ���
		goto NULL_FILE_PTR_ERR;

	write_pos = SECTOR_INC_SPARE_BYTE * PSN; //������ �ϴ� ��ġ
	spare_pos = write_pos + SECTOR_PER_BYTE; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)

	if (src_meta_buffer != NULL) //���� ������ ������ �о���� meta������ ������ �ÿ� meta ���� ������ ���Ͽ� �ٽ� ���� �ʴ´�
	{
		if (src_meta_buffer->get_sector_state() == SECTOR_STATE::EMPTY) //�ش� ����(������)�� ����ִٸ� meta ���� ���� �� ���
		{
			fseek(storage, write_pos, SEEK_SET); //������ �ϴ� ���� ����(������)�� ��ġ�� �̵�
			fwrite(&src_data, sizeof(char), 1, storage); //������ ���(1����Ʈ ũ��)

			src_meta_buffer->set_sector_state(SECTOR_STATE::VALID);

			//1����Ʈ��ŭ ����Ͽ����Ƿ� 511����Ʈ��ŭ ���� Spare area�� �̵� 
			fseek(storage, (SECTOR_PER_BYTE - 1), SEEK_CUR);
			SPARE_write(flashmem, storage, src_meta_buffer); //���ο� meta���� ���

			std::cout << "done" << std::endl;
		}
		else //�ش� ����(������)�� VALID Ȥ�� INVALID 
		{
			std::cout << "overwrite error" << std::endl;
			fclose(storage);

			return COMPLETE;
		}
	}
	else //���� �������� ������ �о���� meta������ �������� ���� �ÿ� meta���� �Ǻ� �� ���� ���� ���� Spare Area �д´�
	{
		fseek(storage, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�
		SPARE_read(flashmem, storage, meta_buffer); //�ش� ����(������)�� meta������ �д´�
		
		if (meta_buffer->get_sector_state() == SECTOR_STATE::EMPTY) //�ش� ����(������)�� ����ִٸ� meta ���� ���� �� ���
		{
			//���� ���� �������� ��ġ�� �а��� �ϴ� ���� ����(������)�� ���� ����(������)�� ���� ��ġ
			fseek(storage, -SECTOR_INC_SPARE_BYTE, SEEK_CUR); //������ �ϴ� ���� ����(������)�� ��ġ�� �ٽ� �̵�
			fwrite(&src_data, sizeof(char), 1, storage); //������ ���(1����Ʈ ũ��)

			meta_buffer->set_sector_state(SECTOR_STATE::VALID);

			//1����Ʈ��ŭ ����Ͽ����Ƿ� 511����Ʈ��ŭ ���� Spare area�� �̵� 
			fseek(storage, (SECTOR_PER_BYTE - 1), SEEK_CUR);
			SPARE_write(flashmem, storage, meta_buffer); //���ο� meta���� ���

			std::cout << "done" << std::endl;
		}
		else
		{
			std::cout << "overwrite error" << std::endl;

			delete meta_buffer;
			fclose(storage);

			return COMPLETE;
		}
	}

	/***
		���� �������� �̹� �о���� meta ������ �����Ͽ� ����� �䱸�Ͽ��� ���, ���� meta ������ �޸� ������ �ش� �������� ����
		���� �������� meta ������ �о ����� �����Ͽ��� ���, ���� meta ������ �޸� ������ ���� �������� ����
	***/
	if (src_meta_buffer == NULL) //���� �������� meta ���� �б� �� ��� ���� �Ͽ��� ���
		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
		
	/*** for Remaining Space Management ***/
	flashmem->v_flash_info.written_sector_count++; //��ϵ� ���� �� ����

	if(storage != NULL)
		fclose(storage);

	return SUCCESS;

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : storage.bin ������ �а� ���� ���� �� �� �����ϴ�. (Flash_write)\n");
	system("pause");
	exit(EXIT_FAILURE);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (Flash_write)\n");
	system("pause");
	exit(EXIT_FAILURE);
}

int Flash_erase(FlashMem*& flashmem, unsigned int PBN) //���� ��Ͽ� �ش��ϴ� �����͸� ����
{
	FILE* storage = NULL;

	char erase_buffer = NULL; //����(������)������ ������ ������ ������� �� �� ������ ��

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	unsigned int erase_start_pos = (SECTOR_INC_SPARE_BYTE * BLOCK_PER_SECTOR) * PBN; //������� �ϴ� ��� ��ġ�� ���� 
	META_DATA** block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ����

	/***
		�ش� ����� ���� ���͵鿡 ���ؼ� ��� erase
		�� ���͵��� Spare area�� �ʱ�ȭ
	***/

	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (PBN > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	if ((storage = fopen("storage.bin", "rb+")) == NULL) //�а� ������ + �������� ��� (���� �б� ���� �� ��� ���ϳ����� ��� �ʱ�ȭ)
		goto NULL_FILE_PTR_ERR;
	
	/*** �ش� ��Ͽ� ���� Erase ���� �� �÷��� �޸��� ������ ���� ���� ***/
	/*** for Remaining Space Management ***/
	SPARE_reads(flashmem, PBN, block_meta_buffer_array); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	update_v_flash_info_for_erase(flashmem, block_meta_buffer_array); //����� �� �ش� ��ϳ��� ��� ����(������)�� ���Ͽ� meta ������ ���� ������ �÷��� �޸� ���� ����

	/*** Deallocate block_meta_buffer_array ***/
	if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	fseek(storage, erase_start_pos, SEEK_SET); //erase�ϰ��� �ϴ� ���� ����� ���� ��ġ�� �̵�

	for (__int8 offset_index =  0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		fwrite(&erase_buffer, sizeof(char), 1, storage); //������ ���� �ʱ�ȭ
		fseek(storage, SECTOR_PER_BYTE - 1, SEEK_CUR); //������ ��� �� 1byte�� ����ϵ��� �Ͽ����Ƿ�, ������ ������ ����(511byte)�� ���� �ǳʶ�

		SPARE_init(flashmem, storage); //Spare area �ʱ�ȭ

#ifdef PAGE_TRACE_MODE //Trace for Per Sector(Page) Wear-leveling
		flashmem->page_trace_info[(erase_start_pos / SECTOR_INC_SPARE_BYTE) + offset_index].erase_count++; //�ش� ����(������)�� ����� ī��Ʈ ����
#endif
	}
	
	/*** trarce���� ���� ��� ***/
	flashmem->v_flash_info.flash_erase_count++; //Global �÷��� �޸� ����� ī��Ʈ ����

#ifdef BLOCK_TRACE_MODE //Trace for Per Block Wear-leveling
	flashmem->block_trace_info[PBN].erase_count++; //�ش� ����� ����� ī��Ʈ ����
#endif

	fclose(storage);

	printf("%u-th block erased\n", PBN);
	return SUCCESS;

NULL_FILE_PTR_ERR:
	fprintf(stderr, "ġ���� ���� : storage.bin ������ �а� ���� ���� �� �� �����ϴ�. (Flash_erase)\n");
	system("pause");
	exit(EXIT_FAILURE);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (Flash_erase)\n");
	system("pause");
	exit(EXIT_FAILURE);
}