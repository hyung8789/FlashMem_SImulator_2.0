#include "FlashMem.h"

// init, read, write, erase �Լ� ����
// �÷��� �޸𸮿� ���� ���������� �����Ͽ� �۾�

int init(FlashMem** flashmem, unsigned short megabytes, int mapping_method, int table_type) //megabytes ũ���� �÷��� �޸𸮸� ����
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

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	//���� �÷��� �޸� ���� �� �� ����
	if (*flashmem != NULL)
	{
		delete (*flashmem); //�������Ͽ� �޸� ����
		(*flashmem) = NULL; //�������Ͽ� �ּ� �ʱ�ȭ
	}
	remove("rr_read_index.txt"); //���� Spare Block Table�� read_index ����
	(*flashmem) = new FlashMem(megabytes); //���� �Ҵ�

	//Spare Block�� �����ϴ� ���ϴ� �����ؾ� �ϴ� ��ü ����(���) ���� ����
	//���͸��� Spare Area�� ����(512+16byte)�Ͽ�, Spare Area�� ������� ����(512byte) ���� ��(sector_size) ��ŭ ������ ��
	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if ((storage = fopen("storage.bin", "wb")) == NULL) //���� + �������� ���
	{
		fprintf(stderr, "storage.bin ������ ������� �� �� �����ϴ�. (init)");
		return FAIL;
	}

	if ((volume = fopen("volume.txt", "wt")) == NULL) //���� + �ؽ�Ʈ���� ���
	{
		fprintf(stderr, "volume.txt ������ ������� �� �� �����ϴ�. (init)");
		if (storage != NULL)
			fclose(storage);
	
		return FAIL;
	}

	/*** ���� ���̺�, Spare ��� ���̺� ���� �� �ʱ�ȭ ***/
	unsigned int spare_block_index = f_flash_info.block_size - 1; //��ü ����� �� �ڿ������� ���������� �ʱ� Spare Block �Ҵ��� ���� ��ü ��� ��-1 
	switch (mapping_method) 
	{
	default:
		break;

	case 2: //��� ����
		block_level_mapping_table = new unsigned int[f_flash_info.block_size - f_flash_info.spare_block_size]; //Spare ��� ���� ������ ��ŭ�� ���� ���̺� ����
		(*flashmem)->spare_block_table = new Spare_Block_Table(f_flash_info.spare_block_size);

		//Spare ����� ��ü ����� �� �ڿ������� ���������� �Ҵ�
		for (unsigned int i = 0; i < f_flash_info.spare_block_size; i++) //Spare ����� �̸� �Ҵ��Ͽ��� ��
		{
			if ((*flashmem)->spare_block_table->seq_write(spare_block_index--) == FAIL)
			{
				fprintf(stderr, "���� : Spare Block Table �ʱ� �Ҵ� ����\n");
				system("pause");
				exit(1);
			}
		}

		//table type�� ���� ���̺� �ʱ�ȭ
		if (table_type == 0) //static table
		{
			for (unsigned int table_index = 0; table_index < f_flash_info.block_size - f_flash_info.spare_block_size; table_index++)
			{
				block_level_mapping_table[table_index] = table_index;
			}
		}
		else //dynamic table
		{
			for (unsigned int table_index = 0; table_index < f_flash_info.block_size - f_flash_info.spare_block_size; table_index++)
			{
				block_level_mapping_table[table_index] = DYNAMIC_MAPPING_INIT_VALUE;
			}
		}
		break;

	case 3: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		log_block_level_mapping_table = new unsigned int*[f_flash_info.block_size - f_flash_info.spare_block_size]; //row : ��ü PBN�� ��
		offset_level_mapping_table = new __int8[f_flash_info.block_size * BLOCK_PER_SECTOR]; //������ ���� ���̺�(Spare Block ����)
		(*flashmem)->spare_block_table = new Spare_Block_Table(f_flash_info.spare_block_size);

		//Spare ����� ��ü ����� �� �ڿ������� ���������� �Ҵ�
		for (unsigned int i = 0; i < f_flash_info.spare_block_size; i++) //Spare ����� �̸� �Ҵ��Ͽ��� ��
		{
			if ((*flashmem)->spare_block_table->seq_write(spare_block_index--) == FAIL)
			{
				fprintf(stderr, "���� : Spare Block Table �ʱ� �Ҵ� ����\n");
				system("pause");
				exit(1);
			}
		}

		for (unsigned int table_index = 0; table_index < f_flash_info.block_size - f_flash_info.spare_block_size; table_index++) //row1:PBN1
		{
			log_block_level_mapping_table[table_index] = new unsigned int[2]; //col : �� ������ ���� PBN1, PBN2�� ��Ÿ��
			log_block_level_mapping_table[table_index][0] = DYNAMIC_MAPPING_INIT_VALUE; //PBN1�� �ش��ϴ� ��ġ
			log_block_level_mapping_table[table_index][1] = DYNAMIC_MAPPING_INIT_VALUE; //PBN2�� �ش��ϴ� ��ġ
		}

		for (unsigned int table_index = 0; table_index < f_flash_info.block_size * BLOCK_PER_SECTOR; table_index++)
		{
			offset_level_mapping_table[table_index] = OFFSET_MAPPING_INIT_VALUE;
		}
	
		break;
	}

	/*** ���̺� �޸𸮿� ĳ�� �� ���� ***/
	//Spare Block ���̺��� ��� �̹� (*flashmem)->spare_block_table->table_array�� �Ҵ�Ǿ���
	switch (mapping_method)
	{
	default:
		break;

	case 2: //��� ���� ���
		//���̺� ����
		(*flashmem)->block_level_mapping_table = block_level_mapping_table;
		break;

	case 3: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		//���̺� ����
		(*flashmem)->log_block_level_mapping_table = log_block_level_mapping_table;
		(*flashmem)->offset_level_mapping_table = offset_level_mapping_table;
		break;
	}
	(*flashmem)->save_table(mapping_method);

	/*** ���� ����� ����� ��� GC�� ���� Victim Block ť ���� ***/
	switch (mapping_method)
	{
	case 0:
		break;

	default:
		(*flashmem)->victim_block_queue = new Victim_Queue(f_flash_info.block_size);
		break;
	}

	/*** Spare Area�� ������ 1���� ũ���� data_inc_spare_array ���� ***/
	data_inc_spare_array = new unsigned char[SECTOR_INC_SPARE_BYTE]; //Spare Area�� ������ ����(528����Ʈ) ũ���� �迭
	memset(data_inc_spare_array, NULL, SECTOR_INC_SPARE_BYTE); //NULL������ ��� �ʱ�ȭ (����Ʈ ����)
	for (int byte_unit = SECTOR_PER_BYTE - 1; byte_unit < SECTOR_INC_SPARE_BYTE - 1; byte_unit++) //���� ��(0~527)�� 511 ~ 527 ���� Spare Area�� ���� �Ҵ�
	{
		data_inc_spare_array[byte_unit] = SPARE_INIT_VALUE; //0xff(16) = 11111111(2) = 255(10) �� �ʱ�ȭ
	}

	//for ftl algorithm
	/*** 
		Spare Block�� ���� 1���� ũ���� spare_block_array ���� 
		0 ~ 511 : Data area
		512 ~ 527 : Spare area
	***/
	if (mapping_method != 0) //���� ��� ��� ��
	{
		spare_block_array = new unsigned char[SECTOR_INC_SPARE_BYTE];
		memset(spare_block_array, NULL, SECTOR_INC_SPARE_BYTE);
		spare_block_array[SECTOR_PER_BYTE] = (0x7f); //0x7f(16) = 01111111(2) = 127(10) �� �ʱ�ȭ (not_spare_block ��Ʈ ��ġ�� 0���� set)

		for (int byte_unit = SECTOR_PER_BYTE + 1; byte_unit < SECTOR_INC_SPARE_BYTE; byte_unit++) //���� ��(0~527)�� 513 ~ 527 ���� Spare Area�� ���� �Ҵ�
		{
			spare_block_array[byte_unit] = SPARE_INIT_VALUE; //0xff(16) = 11111111(2) = 255(10)�� �ʱ�ȭ
		}
	}

	/*** �÷��� �޸� ���丮�� ���� ���� ***/
	init_next_pos = ftell(storage);
	bool flag_write_spare_block = false;

	if (mapping_method == 0) //non-FTL
	{
		while (1) //�Է¹��� MB��ŭ ���Ͽ� ���
		{

			fwrite(data_inc_spare_array, sizeof(unsigned char), SECTOR_INC_SPARE_BYTE, storage); //������ ���� ���� ���
			init_next_pos += SECTOR_INC_SPARE_BYTE;

			printf("%ubytes / %ubytes (%.1f%%)\r", init_next_pos, f_flash_info.storage_byte, ((float)init_next_pos / (float)(f_flash_info.storage_byte)) * 100);
			if (init_next_pos >= f_flash_info.storage_byte) break; //������ ����� ��ġ�� Spare Area�� ������ ��������� �뷮�� ���� ��� ����
		}
	}
	else //for FTL algorithm : Spare ����� �Ҵ�
	{
		while (1) //�Է¹��� MB��ŭ ���Ͽ� ���
		{
			if (flag_write_spare_block != true)
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
				flag_write_spare_block = true; 

			printf("%ubytes / %ubytes (%.1f%%)\r", init_next_pos, f_flash_info.storage_byte, ((float)init_next_pos / (float)f_flash_info.storage_byte) * 100);
			if (init_next_pos >= f_flash_info.storage_byte) break; //������ ����� ��ġ�� Spare Area�� ������ ��������� �뷮�� ���� ��� ����
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

	(*flashmem)->gc->RDY_v_flash_info_for_set_invalid_ratio_threshold = true; //��ȿ�� �Ӱ谪 ������ ���� ������ ���丮�� ���� ���� �Ϸ� �˸�

	return SUCCESS;
}

int Flash_read(FlashMem** flashmem, META_DATA** dst_meta_buffer, unsigned int PSN, char& dst_data) //���� ���Ϳ� �����͸� �о��
{
	FILE* storage = NULL;
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����, FTL �˰����� ���� dst_buffer�� ����

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	unsigned int read_pos = 0; //�а��� �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����
	char read_buffer = NULL; //�о���� ������

	if (*flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (PSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	if ((storage = fopen("storage.bin", "rb")) == NULL) //�б� + �������� ���
	{
		fprintf(stderr, "storage.bin ������ �б���� �� �� �����ϴ�. (Flash_read)");
		return FAIL;
	}

	read_pos = SECTOR_INC_SPARE_BYTE * PSN; //�а��� �ϴ� ���� ����(������)�� ��ġ
	spare_pos = read_pos + SECTOR_PER_BYTE; //�а��� �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)

	if (dst_meta_buffer != NULL) //meta���� ��û �� FTL �Լ��� meta���� ����
	{
		fseek(storage, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�
		meta_buffer = SPARE_read(flashmem, &storage);

		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true) //�ش� ����(������)�� ������� �ʰ�, ��ȿ�ϸ� �д´�
		{
			//���� ���� �������� ��ġ�� �а��� �ϴ� ���� ����(������)�� ���� ����(������)�� ���� ��ġ
			fseek(storage, -SECTOR_INC_SPARE_BYTE, SEEK_CUR); //�а��� �ϴ� ���� ����(������)�� ��ġ�� �ٽ� �̵�
			fread(&read_buffer, sizeof(char), 1, storage); //�ش� ���� ����(������)�� ��ϵ� �� �б�
		}
		else
			
		(*dst_meta_buffer) = meta_buffer;

	}
	else //������ ������ �д´�, �̿� ���� �б� ī��Ʈ ����
	{
		fseek(storage, read_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� ��ġ�� �̵�
		fread(&read_buffer, sizeof(char), 1, storage); //�ش� ���� ����(������)�� ��ϵ� �� �б�

		/*** trace���� ���� ��� ***/
		(*flashmem)->v_flash_info.flash_read_count++; //�÷��� �޸� �б� ī��Ʈ ����

#if BLOCK_TRACE_MODE == 1 //Trace for Per Block Wear-leveling
		(*flashmem)->block_trace_info[PSN / BLOCK_PER_SECTOR].block_read_count++; //�ش� ����� �б� ī��Ʈ ����
#endif
	
	}

	//�о���� ������ ���� ������ ����, ������ NULL ����
	dst_data = read_buffer;

	fclose(storage);

	if (read_buffer != NULL)
		return SUCCESS;
	else 
		return COMPLETE;

}

int Flash_write(FlashMem** flashmem, META_DATA** src_meta_buffer, unsigned int PSN, const char src_data) //���� ���Ϳ� �����͸� ���
{
	FILE* storage = NULL;
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	unsigned int write_pos = 0; //������ �ϴ� ���� ����(������)�� ��ġ
	unsigned int spare_pos = 0; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����
	
	//�̹� �Էµ� ��ġ�� ������ �Է� �õ��� overwrite ���� �߻�

	if (*flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (PSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	if ((storage = fopen("storage.bin", "rb+")) == NULL) //�а� ���� ��� + �������� ���
	{
		fprintf(stderr, "storage.bin ������ �а� ���� ���� �� �� �����ϴ�. (Flash_write)");
		return FAIL;
	}

	write_pos = SECTOR_INC_SPARE_BYTE * PSN; //������ �ϴ� ��ġ
	spare_pos = write_pos + SECTOR_PER_BYTE; //������ �ϴ� ���� ����(������)�� Spare Area ���� ����(������ ������ �ǳʶ�)

	if (src_meta_buffer != NULL) //������ �о���� meta������ ������ �ÿ� �ٽ� ���� �ʴ´�
	{
		if ((*src_meta_buffer)->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true) //�ش� ����(������)�� ����ִٸ� ���
		{
			fseek(storage, write_pos, SEEK_SET); //������ �ϴ� ���� ����(������)�� ��ġ�� �̵�
			fwrite(&src_data, sizeof(char), 1, storage); //������ ���(1����Ʈ ũ��)

			(*src_meta_buffer)->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] = false;

			//1����Ʈ��ŭ ����Ͽ����Ƿ� 511����Ʈ��ŭ ���� Spare area�� �̵� 
			fseek(storage, (SECTOR_PER_BYTE - 1), SEEK_CUR);
			SPARE_write(flashmem, &storage, src_meta_buffer); //���ο� meta���� ���

			std::cout << "done" << std::endl;
		}
		else
		{
			std::cout << "overwrite error" << std::endl;
			fclose(storage);

			return COMPLETE;
		}
	}
	else //������ �о���� meta������ �������� ���� �ÿ� meta���� �Ǻ� �� ���� ���� ���� Spare Area �д´�
	{
		fseek(storage, spare_pos, SEEK_SET); //�а��� �ϴ� ���� ����(������)�� Spare Area ���� �������� �̵�
		meta_buffer = SPARE_read(flashmem, &storage); //�ش� ����(������)�� meta������ �д´�
		
		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true) //�ش� ����(������)�� ����ִٸ� ���
		{
			//���� ���� �������� ��ġ�� �а��� �ϴ� ���� ����(������)�� ���� ����(������)�� ���� ��ġ
			fseek(storage, -SECTOR_INC_SPARE_BYTE, SEEK_CUR); //������ �ϴ� ���� ����(������)�� ��ġ�� �ٽ� �̵�
			fwrite(&src_data, sizeof(char), 1, storage); //������ ���(1����Ʈ ũ��)

			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] = false; //������� �ʴ� ���ͷ� �ٲ۴�

			//1����Ʈ��ŭ ����Ͽ����Ƿ� 511����Ʈ��ŭ ���� Spare area�� �̵� 
			fseek(storage, (SECTOR_PER_BYTE - 1), SEEK_CUR);
			SPARE_write(flashmem, &storage, &meta_buffer); //���ο� meta���� ���

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

	/*** for Remaining Space Management ***/
	(*flashmem)->v_flash_info.written_sector_count++; //��ϵ� ���� �� ����

	if(storage != NULL)
		fclose(storage);

	if(meta_buffer != NULL)
		delete meta_buffer;

	return SUCCESS;
}

int Flash_erase(FlashMem** flashmem, unsigned int PBN) //���� ��Ͽ� �ش��ϴ� �����͸� ����
{
	FILE* storage = NULL;

	char erase_buffer = NULL; //����(������)������ ������ ������ ������� �� �� ������ ��

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	unsigned int erase_start_pos = (SECTOR_INC_SPARE_BYTE * BLOCK_PER_SECTOR) * PBN; //������� �ϴ� ��� ��ġ�� ���� 
	META_DATA** block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� Spare Area�κ��� ���� �� �ִ� META_DATA Ŭ���� �迭 ����

	//�ش� ����� ���� ���͵鿡 ���ؼ� ��� erase
	//�� ���͵��� Spare area�� �ʱ�ȭ

	if (*flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (PBN > (unsigned int)((MB_PER_BLOCK * f_flash_info.flashmem_size) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	if ((storage = fopen("storage.bin", "rb+")) == NULL) //�а� ������ + �������� ���(���� �б� ���� �� ��� ���ϳ����� ��� �ʱ�ȭ)
	{
		fprintf(stderr, "storage.bin ������ �а� ���� ���� �� �� �����ϴ�. (Flash_erase)");
		return FAIL;
	}
	
	/*** �ش� ��Ͽ� ���� Erase ���� �� �÷��� �޸��� ������ ���� ���� ***/
	/*** for Remaining Space Management ***/
	block_meta_buffer_array = SPARE_reads(flashmem, PBN); //�ش� ����� ��� ����(������)�� ���� meta������ �о��
	if(update_v_flash_info_for_erase(flashmem, block_meta_buffer_array) != SUCCESS)
	{
		fprintf(stderr, "���� : nullptr (block_meta_buffer_array)");
		system("pause");
		exit(1);
	}

	/*** Deallocate block_meta_buffer_array ***/
	if (deallocate_block_meta_buffer_array(block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	fseek(storage, erase_start_pos, SEEK_SET); //erase�ϰ��� �ϴ� ���� ����� ���� ��ġ�� �̵�

	for (__int8 offset_index =  0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		fwrite(&erase_buffer, sizeof(char), 1, storage); //������ ���� �ʱ�ȭ

		/*** ������ ��� �� 1byte�� ����ϵ��� �Ͽ����Ƿ�, ������ 511byte������ ���ؼ��� ���� ó���� ���Ͽ� �ǳʶڴ� ***/
		fseek(storage, SECTOR_PER_BYTE - 1, SEEK_CUR); //������ ������ ����(511byte)�� ���� �ǳʶ�
		if (SPARE_init(flashmem, &storage) != SUCCESS) //Spare area �ʱ�ȭ
		{
			fprintf(stderr, "���� : SPARE_init");
			system("pause");
			exit(1);
		}
	}
	
	/*** trarce���� ���� ��� ***/
	(*flashmem)->v_flash_info.flash_erase_count++; //�÷��� �޸� ����� ī��Ʈ ����

#if BLOCK_TRACE_MODE == 1 //Trace for Per Block Wear-leveling
	(*flashmem)->block_trace_info[PBN].block_erase_count++; //�ش� ����� ����� ī��Ʈ ����
#endif

	fclose(storage);

	printf("%u-th block erased\n", PBN);
	return SUCCESS;

MEM_LEAK_ERR:
	fprintf(stderr, "���� : meta ������ ���� �޸� ���� �߻� (Flash_erase)\n");
	system("pause");
	exit(1);
}