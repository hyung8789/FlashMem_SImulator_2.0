#include "FlashMem.h"

// Print_table, FTL_read, FTL_write, trace ����
// �� ���� ��ȣ �Ǵ� �� ��� ��ȣ�� �������̺�� �����Ͽ� physical_func���� read, write, erase�� �����Ͽ� �۾��� ����

int Print_table(FlashMem** flashmem, int mapping_method, int table_type)  //���� ���̺� ���
{
	FILE* table_output = NULL; //���̺��� ���Ϸ� ����ϱ� ���� ���� ������

	unsigned int block_table_size = 0; //��� ���� ���̺��� ũ��
	unsigned int spare_table_size = 0; //Spare block ���̺��� ũ��
	unsigned int offset_table_size = 0; //������ ���� ���̺��� ũ��

	unsigned int table_index = 0; //���� ���̺� �ε���

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	if (*flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�


	if ((table_output = fopen("table.txt", "wt")) == NULL)
	{
		fprintf(stderr, "table.txt ������ ������� �� �� �����ϴ�.\n");

		return FAIL;
	}

	switch (mapping_method) //���� ��Ŀ� ���� ���� ���̺� ��� ����
	{
	case 1:
		break;

	case 2: //��� ����
		block_table_size = f_flash_info.block_size - f_flash_info.spare_block_size; //�Ϲ� ��� ��
		spare_table_size = f_flash_info.spare_block_size; //Spare block ��

		std::cout << "< Block Mapping Table (LBN -> PBN) >" << std::endl;
		fprintf(table_output, "< Block Mapping Table (LBN -> PBN) >\n");

		while (table_index < block_table_size)
		{
			//�Ҵ�� ���鸸 ���
			if ((*flashmem)->block_level_mapping_table[table_index] == DYNAMIC_MAPPING_INIT_VALUE)
			{
				table_index++;
			}
			else
			{
				printf("%u -> %u\n", table_index, (*flashmem)->block_level_mapping_table[table_index]);
				fprintf(table_output, "%u -> %u\n", table_index, (*flashmem)->block_level_mapping_table[table_index]);

				table_index++;
			}
		}

		std::cout << "\n< Spare Block Table (LBN -> PBN) >" << std::endl;
		fprintf(table_output, "\n< Spare Block Table (LBN -> PBN) >\n");
		table_index = 0;
		while (table_index < spare_table_size)
		{
			printf("%u -> %u\n", table_index, (*flashmem)->spare_block_table->table_array[table_index]);
			fprintf(table_output, "%u -> %u\n", table_index, (*flashmem)->spare_block_table->table_array[table_index]);

			table_index++;
		}

		break;

	case 3: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		block_table_size = f_flash_info.block_size - f_flash_info.spare_block_size; //�Ϲ� ��� ��
		spare_table_size = f_flash_info.spare_block_size; //Spare block ��
		offset_table_size = f_flash_info.block_size * BLOCK_PER_SECTOR; //������ ���̺��� ũ��

		std::cout << "< Hybrid Mapping Block level Table (LBN -> PBN1, PBN2) >" << std::endl;
		fprintf(table_output, "< Hybrid Mapping Block level Table (LBN -> PBN1, PBN2) >\n");

		while (table_index < block_table_size)
		{
			//�Ҵ�� ���鸸 ���
			if ((*flashmem)->log_block_level_mapping_table[table_index][0] == DYNAMIC_MAPPING_INIT_VALUE &&
				(*flashmem)->log_block_level_mapping_table[table_index][1] == DYNAMIC_MAPPING_INIT_VALUE) //PBN1�� PBN2�� �Ҵ���� ���� ���
			{
				table_index++;
			}
			else if ((*flashmem)->log_block_level_mapping_table[table_index][0] != DYNAMIC_MAPPING_INIT_VALUE &&
				(*flashmem)->log_block_level_mapping_table[table_index][1] == DYNAMIC_MAPPING_INIT_VALUE) //PBN1 �Ҵ� PBN2�� �Ҵ���� ���� ���
			{
				printf("%u -> %u, non-assigned\n", table_index, (*flashmem)->log_block_level_mapping_table[table_index][0]);
				fprintf(table_output, "%u -> %u, non-assigned\n", table_index, (*flashmem)->log_block_level_mapping_table[table_index]);

				table_index++;
			}
			else if ((*flashmem)->log_block_level_mapping_table[table_index][0] == DYNAMIC_MAPPING_INIT_VALUE &&
				(*flashmem)->log_block_level_mapping_table[table_index][1] != DYNAMIC_MAPPING_INIT_VALUE) //PBN1�� �Ҵ���� �ʰ� PBN2�� �Ҵ� �� ���
			{
				printf("%u -> non-assigned, %u\n", table_index, (*flashmem)->log_block_level_mapping_table[table_index][1]);
				fprintf(table_output, "%u -> non-assigned, %u\n", table_index, (*flashmem)->log_block_level_mapping_table[table_index][1]);

				table_index++;
			}
			else //PBN1, PBN2 ��� �Ҵ� �� ���
			{
				printf("%u -> %u, %u\n", table_index, (*flashmem)->log_block_level_mapping_table[table_index][0], (*flashmem)->log_block_level_mapping_table[table_index][1]);
				fprintf(table_output, "%u -> %u, %u\n", table_index, (*flashmem)->log_block_level_mapping_table[table_index][0], (*flashmem)->log_block_level_mapping_table[table_index][1]);

				table_index++;
			}
		}

		std::cout << "\n< Spare Block Table (LBN -> PBN) >" << std::endl;
		fprintf(table_output, "\n< Spare Block Table (LBN -> PBN) >\n");
		table_index = 0;
		while (table_index < spare_table_size)
		{
			printf("%u -> %u\n", table_index, (*flashmem)->spare_block_table->table_array[table_index]);
			fprintf(table_output, "%u -> %u\n", table_index, (*flashmem)->spare_block_table->table_array[table_index]);

			table_index++;
		}

		std::cout << "\n< Hybrid Mapping Offset level Table (Index -> POffset) >" << std::endl;
		fprintf(table_output, "\n< Hybrid Mapping Offset level Table (Index -> POffset) >\n");
		table_index = 0;
		while (table_index < offset_table_size)
		{
			//�Ҵ�� ���鸸 ���
			if ((*flashmem)->offset_level_mapping_table[table_index] == OFFSET_MAPPING_INIT_VALUE)
			{
				table_index++;
			}
			else
			{
				printf("%u -> %d\n", table_index, (*flashmem)->offset_level_mapping_table[table_index]);
				fprintf(table_output, "%u -> %d\n", table_index, (*flashmem)->offset_level_mapping_table[table_index]);
 				table_index++;
			}
		}
		break;

	default:
		fclose(table_output);
		return FAIL;
	}

	fclose(table_output);
	std::cout << ">> table.txt�� ��ϵ�" << std::endl;

	return SUCCESS;
}

int FTL_read(FlashMem** flashmem, unsigned int LSN, int mapping_method, int table_type) //////�� ���� �Ǵ� �� ��Ͽ� �ش�Ǵ� �������̺� �� ���� ���� �Ǵ� ���� ����� ��ġ�� ��ȯ
{
	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE; //�ش� ���� ���Ͱ� ��ġ�ϰ� �ִ� ���� ���
	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ
	unsigned int offset_level_table_index = DYNAMIC_MAPPING_INIT_VALUE; //������ ���� ���̺� �ε���

	__int8 Loffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ���� ���� Offset = 0 ~ 31
	__int8 Poffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ������ ���� Offset = 0 ~ 31

	char read_buffer = NULL; //���� ���ͷκ��� �о���� ������

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	if (*flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (LSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	switch (mapping_method)
	{
	case 1:
		break;

	case 2: //��� ����
		LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
		PBN = (*flashmem)->block_level_mapping_table[LBN];
		Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
		PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //������ ����� ���� ���� ��ȣ

		if (PBN == DYNAMIC_MAPPING_INIT_VALUE) //Dynamic Table �ʱⰪ�� ���
			goto NON_ASSIGNED_TABLE;
		else
		{
			/*** ��ȿȭ�� ��� �Ǻ� - GC�� ���� ���� ó������ �ʾ��� ��� ***/
			if (PSN % BLOCK_PER_SECTOR == 0) //���� ��ġ�� ����� ù ��° ������ ���
			{
				meta_buffer = SPARE_read(flashmem, PSN); //Spare ������ ����
				switch (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block])
				{
				case true: //��ȿ�� ����� ���
					break;

				case false: //��ȿ���� ���� ��� - GC�� ���� ���� ó���� �����ʾ��� ���
					goto INVALID_BLOCK;
					break;
				}
			}
			else //����� ù ��° ������ Spare ������ ���� �ش� ����� ��ȿȭ �Ǻ�
			{
				meta_buffer = SPARE_read(flashmem, (PBN * BLOCK_PER_SECTOR)); //����� ù ��° ������ Spare ������ ����
				switch (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block])
				{
				case true: //��ȿ�� ����� ���
					delete meta_buffer;
					meta_buffer = SPARE_read(flashmem, PSN); //���� ��ġ�� Spare ������ ����
					break;

				case false: //��ȿ���� ���� ��� - GC�� ���� ���� ó���� �����ʾ��� ���
					goto INVALID_BLOCK;
					break;
				}
			}

			/***
				��� ������ ��� Overwrite�� ���� ���� ����(������)�� ��ȿȭ�� �ÿ� �ش� ����� �׻� ��ȿȭ�ȴ�.
				�̿� ����, GC�� ���� ��ȿȭ�� ����� Erase ����Ǿ���, ��� ���� ���̺� �󿡼� ���� ����� �Ҵ�Ǿ����� ���,
				�׻� ��� ���� ��� �����ʹ� ��ȿ�ϴ�. (��ϵǾ� �ְų�, ����ְų�)
			***/

#if DEBUG_MODE == 1
			switch (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector])
			{
			case true:
				break;

			case false: //�׻� ��ȿ�Ͽ��� ��
				goto WRONG_META_ERR;
			}
#endif

			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true) //�� ������ ���
				goto EMPTY_PAGE;
			else
			{
				Flash_read(flashmem, NULL, PSN, read_buffer); //�����͸� �о��
				goto OUTPUT_DATA_SUCCESS;
			}
		}

		break;

	case 3: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		/***
			PBN1 : �������� �׻� ��ġ��Ŵ
			PBN2 : ������ ���� ���̺��� ���Ͽ� �о����
			---
			=> ������ ���� �ÿ� ���� PBN1�� �о� �ش� ��ġ�� ����ְų� �ش� �����Ͱ� ��ȿȭ�Ǿ����� ��� PBN2���� ������ ���̺��� ���� �о���δ�.
		***/
		LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
		/*** ���� PBN1 ��ġ�� �д´� ***/
		PBN = (*flashmem)->log_block_level_mapping_table[LBN][0]; //PBN1
		Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
		PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //������ ����� ���� ���� ��ȣ

		if (PBN != DYNAMIC_MAPPING_INIT_VALUE) //PBN1�� �Ҵ�� ���
		{
			meta_buffer = SPARE_read(flashmem, PSN);

			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true) //��ȿ���� ���� ������ ���
			{

#if DEBUG_MODE == 1
				switch (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector])
				{
				case true: //��ȿ���� ������, �����Ͱ� ��ϵǾ� �����Ƿ�, �� �����̸� �ȵ�
					goto WRONG_META_ERR;

				case false:
					break;
				}
#endif
				delete meta_buffer;
				meta_buffer = NULL;

				//PBN1�� �Ҵ�Ǿ� ������, ��ȿ���� ���� �������̹Ƿ� PBN2�� �д´�
				goto READ_PBN2;
			}
			else //PBN1���� �ش� ��ġ�� ��ȿ�� ��� (��� ���� ���� ��ġ �Ǵ� ��ϵ� ��ġ)
			{
				if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true)
					goto EMPTY_PAGE; //��ϵ��� ���� ��ġ

				Flash_read(flashmem, NULL, PSN, read_buffer);
				goto OUTPUT_DATA_SUCCESS;
			}
		}
		else //PBN1�� ����ִ� ��� (�Ҵ���� ���� ���)
		{
		READ_PBN2: //PBN1�� �Ҵ�Ǿ� ���� �ʰų�, ��ȿ �������� ��� PBN2�� ���Ͽ� �д´�

			//PBN2�� �ش��ϴ� ���� ����
			PBN = (*flashmem)->log_block_level_mapping_table[LBN][1]; //PBN2
			
			if (PBN == DYNAMIC_MAPPING_INIT_VALUE) //�Ҵ� ���� ���� ������ ���
				goto NON_ASSIGNED_TABLE;
			
			/***
				�Ϸķ� �� ������ ���̺�(Loffset -> Poffset)���� �ش� ��ġ�� ã���� �Ѵّm,
				(PBN * ��ϴ� ���ͼ�) + (LSN % ��ϴ� ���ͼ�)
				�ش� ���� ����� ������ ���̺�󿡼��� ���� ��ġ + ������ ���̺� �������� ��ġ
			***/

			Loffset = LSN % BLOCK_PER_SECTOR; //������ ���� ���̺� �������� LSN�� �� ������
			offset_level_table_index = (PBN * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
			Poffset = (*flashmem)->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
			PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //������ ����� ���� ���� ��ȣ

			if (Poffset == OFFSET_MAPPING_INIT_VALUE) //��� ���� ���� ������ ���
				goto EMPTY_PAGE;

			/***
				PBN2�� ������ ���̺��� �ʱ� �� OFFSET_MAPPING_INIT_VALUE�� �Ҵ�Ǿ��ְ�,
				�׻� ��ȿ�ϰ� ������� ���� ��ġ�� ����Ų��.
				---
				=> �̿� ����, PBN2�� ������ ���̺��� �ʱ� ���� �ƴ϶��, �� ��ġ�� �׻� ��ȿ�ϰ� ������� �ʴ�.
			***/

#if DEBUG_MODE == 1
			meta_buffer = SPARE_read(flashmem, PSN);

			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true ||
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true) //��ȿ���� �ʰų� �� ������ ���
				goto WRONG_META_ERR; //�߸��� meta����
			else
			{
				delete meta_buffer;
				meta_buffer = NULL;
			}
#endif

			Flash_read(flashmem, NULL, PSN, read_buffer);
			goto OUTPUT_DATA_SUCCESS;
		}

		break;

	default:
		return FAIL;
	}

OUTPUT_DATA_SUCCESS:
	if (meta_buffer != NULL)
		delete meta_buffer;
	std::cout << "PSN : " << PSN << std::endl; //���� ���� ��ȣ ���
	std::cout << read_buffer << std::endl;

	return SUCCESS;

NON_ASSIGNED_TABLE:
	if (meta_buffer != NULL)
		delete meta_buffer;
	std::cout << "no data (Non-Assigned Block in Table)" << std::endl;

	return COMPLETE;

EMPTY_PAGE:
	if (meta_buffer != NULL)
		delete meta_buffer;
	std::cout << "no data (Empty Page)" << std::endl;

	return COMPLETE;

INVALID_PAGE:
	if (meta_buffer != NULL)
		delete meta_buffer;
	std::cout << "no data (Invalid Page)" << std::endl;

	return COMPLETE;

INVALID_BLOCK:
	if (meta_buffer != NULL)
		delete meta_buffer;
	std::cout << "no data (Invalid Block)" << std::endl;

	return COMPLETE;

WRONG_META_ERR:
	fprintf(stderr, "���� : �߸��� meta����\n");
	system("pause");
	exit(1);
}

int FTL_write(FlashMem** flashmem, unsigned int LSN, const char src_data, int mapping_method, int table_type) //�� ���� �Ǵ� �� ��Ͽ� �ش�Ǵ� �������̺� �� ���� ���� �Ǵ� ���� ��� ��ġ�� ���
{
	char block_read_buffer[BLOCK_PER_SECTOR] = { NULL }; //�� ��ϳ��� ������ �ӽ� ���� ����
	__int8 read_buffer_index = 0; //�����͸� �о �ӽ������ϱ� ���� read_buffer �ε��� ����
	
	unsigned int empty_spare_block = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare ���
	unsigned int spare_block_table_index = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare �����  Spare ��� ���̺� �� �ε���
	unsigned int tmp = DYNAMIC_MAPPING_INIT_VALUE; //���̺� SWAP���� �ӽ� ����

	//static table :spare ��� ���
	//unsigned int empty_LBN_for_SWAP = DYNAMIC_MAPPING_INIT_VALUE; //���̺� SWAP���� �� �� ���
	unsigned int empty_spare_PBN_for_valid_data_copy = DYNAMIC_MAPPING_INIT_VALUE; //Overwrite �� ��ȿ ������ copy�� ���� �� ���� ��� (Spare Block ���)

	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE; //�ش� ���� ���Ͱ� ��ġ�ϰ� �ִ� ���� ���
	unsigned int PBN1 = DYNAMIC_MAPPING_INIT_VALUE; //�ش� ���� ���Ͱ� ��ġ�ϰ� �ִ� ���� ���(PBN1)
	unsigned int PBN2 = DYNAMIC_MAPPING_INIT_VALUE; //�ش� ���� ���Ͱ� ��ġ�ϰ� �ִ� ���� ���(PBN2)
	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ
	unsigned int offset_level_table_index = DYNAMIC_MAPPING_INIT_VALUE; //������ ���� ���̺� �ε���

	__int8 Loffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ���� ���� Offset = 0 ~ 31
	__int8 Poffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ������ ���� Offset = 0 ~ 31

	char read_buffer = NULL; //���� ���ͷκ��� �о���� ������
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	if (*flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = (*flashmem)->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	if (LSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	switch (mapping_method) //���� ��Ŀ� ���� �ش� ó�� ��ġ�� �̵�
	{
	case 2: //��� ����
		if (table_type == 0) //��� ���� Static Table
			goto BLOCK_MAPPING_STATIC;

		else if (table_type == 1) //��� ���� Dynamic Table
			goto BLOCK_MAPPING_DYNAMIC;

		else 
			goto WRONG_TABLE_TYPE_ERR;

	case 3: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		goto HYBRID_LOG_DYNAMIC;

	default:
		goto WRONG_TABLE_TYPE_ERR;
	}

BLOCK_MAPPING_STATIC: //��� ���� Static Table
	//����ڰ� �Է��� LSN���� LBN�� ���ϰ� �����Ǵ� PBN�� ���� ���� ��ȣ�� ����
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
	PBN = (*flashmem)->block_level_mapping_table[LBN]; //������ ����� ���� ��� ��ȣ

#if DEBUG_MODE == 1
	//�׻� �����Ǿ� �־�� ��
	if (PBN == DYNAMIC_MAPPING_INIT_VALUE)
		goto WRONG_STATIC_TABLE_ERR;
#endif

	PSN = (PBN * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������
	meta_buffer = SPARE_read(flashmem, PSN); //Spare ������ ����

	/***
		�ش� ����� ��ȿȭ�� ���
		---
		����� ��ȿȭ�� ����(��, �̹� ��ϵ� ��ġ�� ���� Overwrite�� �߻�)�� Spare Block�� �̿��� ��ȿ ������ copy �� SWAP ����
		=> ����, �� ���� �߻����� ����
	***/
	if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] == false)
	{

#if DEBUG_MODE == 1
		//�Ϲ� ��� ���̺� ���Ͽ� Spare Block�� �����Ǿ� �ְų�, ��ȿ�� ����̸�, ����־�� �ȵȴ�.
		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] == true ||
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] == false)
			goto WRONG_META_ERR;
#endif
		delete meta_buffer;
		meta_buffer = NULL;
	}
	/***
		�ش� ����� ��ȿȭ���� ���� ���
		---
		1) �ش� ����� ����ִ� ���
		2) �ش� ����� ������� ���� ���
			2-1) �ش� ������ ��ġ�� �� ���
			2-2) �ش� ������ ��ġ�� ������� ���� ��� (Overwrite)
	***/
	else
	{
		/*** �ش� ����� ����ִ� ��� ***/
		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] == true)
		{
			PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

			if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ��� �� ��� ���� ����
			{
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
				
				//�ش� ������ ��ġ�� ���
				goto BLOCK_MAPPING_COMMON_WRITE_PROC;
			}
			else //��� �� ��ġ�� ����� ù ��° ���Ͱ� �ƴϸ� �� ��� ������ ���� ����
			{
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
				SPARE_write(flashmem, (PBN * BLOCK_PER_SECTOR), &meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 
				
				delete meta_buffer;
				meta_buffer = NULL;

				//�ش� ������ ��ġ�� ���
				goto BLOCK_MAPPING_COMMON_WRITE_PROC;
			}
		}
		/*** �ش� ����� ������� ���� ��� ***/
		else
		{
			delete meta_buffer;
			meta_buffer = NULL;

			PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ
			meta_buffer = SPARE_read(flashmem, PSN);

			/*** �ش� ������ ��ġ�� �� ��� ***/
			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true)
			{

#if DEBUG_MODE == 1
				//����ִµ� ��ȿ���� ������
				if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true)
					goto WRONG_META_ERR; //�߸��� meta ���� ����
#endif

				//�ش� ������ ��ġ�� ���
				goto BLOCK_MAPPING_COMMON_WRITE_PROC;
			}
			/*** �ش� ������ ��ġ�� ������� ���� ��� ***/
			else
				//���� ������ ��ġ�� meta ������ ����Ͽ� ��� ���� ���� Overwrite ó�� ��ƾ���� �̵�
				goto BLOCK_MAPPING_COMMON_OVERWRITE_PROC;
		}
	}

BLOCK_MAPPING_DYNAMIC: //��� ���� Dynamic Table
	//����ڰ� �Է��� LSN���� LBN�� ���ϰ� �����Ǵ� PBN�� ���� ���� ��ȣ�� ����
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
	PBN = (*flashmem)->block_level_mapping_table[LBN]; //������ ����� ���� ��� ��ȣ

	/*** LBN�� PBN�� �������� ���� ��� ***/
	if (PBN == DYNAMIC_MAPPING_INIT_VALUE)
	{
		if (search_empty_block(flashmem, PBN, &meta_buffer, mapping_method, table_type) != SUCCESS) //�� �Ϲ� ���(PBN)�� ���������� Ž���Ͽ� PBN ��, �ش� PBN�� meta���� �޾ƿ´�
			goto END_NO_SPACE; //��� ���� ���� ����

		(*flashmem)->block_level_mapping_table[LBN] = PBN;

		PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

		if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ��� �� ��� ���� ����
		{
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;

			//�ش� ������ ��ġ�� ���
			goto BLOCK_MAPPING_COMMON_WRITE_PROC;
		}
		else //��� �� ��ġ�� ����� ù ��° ���Ͱ� �ƴϸ� �� ��� ������ ���� ����
		{
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
			SPARE_write(flashmem, (PBN * BLOCK_PER_SECTOR), &meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 

			delete meta_buffer;
			meta_buffer = NULL;

			//�ش� ������ ��ġ�� ���
			goto BLOCK_MAPPING_COMMON_WRITE_PROC;
		}
	}

	/*** ����� ù ��° �������� Spare������ ���� �ش� ��� ���� �Ǻ� �� ���� ���� ***/
	PSN = (PBN * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������
	meta_buffer = SPARE_read(flashmem, PSN); //Spare ������ ����

	/*** 
		�ش� ����� ��ȿȭ�� ��� , �ش� ����� ����ִ� ���
		---
		Dynamic Table�� ��� �׻� ��ȿȭ�������� ��ϸ� ������Ų��. 
		�̿� ����, ��ȿȭ���� ���� ����� �����Ǵ� ���� �߻����� �ʴ´�.
		����, �ʱ� ��� �������� ����� �Ҵ��ϹǷ� �׻� ��� ���� ����̴�.
	***/
#if DEBUG_MODE == 1
	if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] == false || 
		meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] == true)
		goto WRONG_META_ERR; //�߸��� meta ����
#endif

	/***
		�ش� ����� ��ȿȭ���� ���� ���
		---
		1) �ش� ����� ����ִ� ��� => �߻����� ����
		2) �ش� ����� ������� ���� ���
			2-1) �ش� ������ ��ġ�� �� ���
			2-2) �ش� ������ ��ġ�� ������� ���� ��� (Overwrite)
	***/

	/*** �ش� ����� ������� ���� ��� ***/
	delete meta_buffer;
	meta_buffer = NULL;

	PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ
	meta_buffer = SPARE_read(flashmem, PSN);

	/*** �ش� ������ ��ġ�� �� ��� ***/
	if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true)
	{

#if DEBUG_MODE == 1
		//����ִµ� ��ȿ���� ������
		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true)
			goto WRONG_META_ERR; //�߸��� meta ���� ����
#endif

		//�ش� ������ ��ġ�� ���
		goto BLOCK_MAPPING_COMMON_WRITE_PROC;
	}
	/*** �ش� ������ ��ġ�� ������� ���� ��� ***/
	else
	{
		//���� ������ ��ġ�� meta ������ ����Ͽ� ��� ���� ���� Overwrite ó�� ��ƾ���� �̵�
		goto BLOCK_MAPPING_COMMON_OVERWRITE_PROC;
	}


BLOCK_MAPPING_COMMON_WRITE_PROC: //��� ���� ���� ó�� ��ƾ 1 : ���ǰ� �ִ� ����� ��� �ִ� �����¿� ���� ��� ����
	PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ
	
	if(meta_buffer == NULL)
		meta_buffer = SPARE_read(flashmem, PSN);

#if DEBUG_MODE == 1
	if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true)
		goto WRONG_META_ERR; //�߸��� meta ���� ����
#endif

	//�ش� ������ ��ġ�� ���
	if (Flash_write(flashmem, &meta_buffer, PSN, src_data) == COMPLETE)
		goto OVERWRITE_ERR;
	
	delete meta_buffer;
	meta_buffer = NULL;

	goto END_SUCCESS; //����

BLOCK_MAPPING_COMMON_OVERWRITE_PROC: //��� ���� ���� ó�� ��ƾ 2 : ���ǰ� �ִ� ����� ���� ��ġ�� ���� Overwrite ����
	/***
		1) �ش� PBN�� ��ȿ�� ������(���ο� �����Ͱ� ��ϵ� ��ġ ����) �� ���ο� �����͸� ������ �� Spare Block�� ����Ͽ� 
		��ȿ ������(���ο� �����Ͱ� ��ϵ� ��ġ ����) copy �� ���ο� ������ ���
		2) ���� PBN ���� ��� ���� �� �ش� ��� ��ȿȭ
		3) ���� PBN�� ���� Spare Block ���̺� �� ��ü �� ���� PBN�� Victim Block���� ����
	***/

	delete meta_buffer;
	meta_buffer = NULL;

	//��ȿ ������ ���� (Overwrite�� ��ġ �� �� ��ġ�� ����) �� ���� ��� ��ȿȭ, ��� ���� ��� ���� ��ȿȭ
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		PSN = (PBN * BLOCK_PER_SECTOR) + offset_index; //�ش� ��Ͽ� ���� �ε���

		meta_buffer = SPARE_read(flashmem, PSN);
		
		if (PSN == (PBN * BLOCK_PER_SECTOR) + Poffset || meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true) //Overwrite�� ���� ��ġ �Ǵ� �� ��ġ
		{
			//�������� ���߱� ���Ͽ� ��� ������ ���ۿ� �� �������� ���
			block_read_buffer[offset_index] = NULL;
		}
		else
			Flash_read(flashmem, NULL, PSN, block_read_buffer[offset_index]);

		/*** meta ���� ���� : ��� �� �ش� ��� ���� ���� ��ȿȭ ***/
		if (PSN % BLOCK_PER_SECTOR == 0) //ù ��° ���Ͷ�� ��� ���� �߰��� ����
		{
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = false; //Spare Block�� SWAP ���� meta ���� �̸� ����
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] = false;
			
			//����ִ� ���Ͱ� �ƴϸ� �ش� ���� ��ȿȭ
			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == false)
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;

			SPARE_write(flashmem, PSN, &meta_buffer);
		}
		else
		{
			//����ִ� ���Ͱ� �ƴϸ� �ش� ���� ��ȿȭ
			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == false)
			{
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;
				SPARE_write(flashmem, PSN, &meta_buffer);
			}
			//��������� �ƹ��͵� ���� �ʴ´�.
		}

		delete meta_buffer;
		meta_buffer = NULL;
	}

	//��ȿȭ�� PBN�� Victim Block���� ���� ���� ���� ���� 
	update_victim_block_info(flashmem, false, PBN, mapping_method);

	/*** �� Spare ����� ã�Ƽ� ��� ***/
	(*flashmem)->spare_block_table->rr_read(empty_spare_block, spare_block_table_index);
	
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		PSN = (empty_spare_block * BLOCK_PER_SECTOR) + offset_index; //���������� ��� ���� �� �������� ���� �ε���
		
		if (block_read_buffer[offset_index] != NULL) //��� ������ �о���� ���ۿ��� �ش� ��ġ�� ������� ������, �� ��ȿ�� �������̸�
		{
			meta_buffer = SPARE_read(flashmem, PSN); //����(������)�� Spare ���� �б�

			if (PSN % BLOCK_PER_SECTOR == 0) //���� ��� �� ��ġ�� ù ��° ���Ͷ��
			{
				//�ش� ����� �Ϲ� ���ȭ �� ��, ���� �� ���, �� ���� ���� ����
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = true;
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
			
				if (Flash_write(flashmem, &meta_buffer, PSN, block_read_buffer[offset_index]) == COMPLETE)
					goto OVERWRITE_ERR;
			}
			else
			{
				/*****
				1mb�� dynamic table type block mapping ����
				��� ���� ������ ��� �� ��(trace_1888.txt),
				w 0 a
				w 1 a
				w 2 a
				w 3 a
				w 4 a => victim block ť ó���Ǿ� ����ִ� ����
				w 5 a => overwrite ���� �߻�(������ �ε��� 31)
				*****/
				if (offset_index == 31)
					system("pause");

				//���� �߻� ��ġ
				if (Flash_write(flashmem, &meta_buffer, PSN, block_read_buffer[offset_index]) == COMPLETE)
					goto OVERWRITE_ERR;
			}

			delete meta_buffer;
			meta_buffer = NULL;
		}
		else if (offset_index == Poffset) //��� ���� ���۰� ����ְ�, ��� �� ��ġ�� Overwrite �� ��ġ�� ���ο� �����ͷ� ���
		{
			meta_buffer = SPARE_read(flashmem, PSN); //����(������)�� Spare ���� �б�
			
			if (PSN % BLOCK_PER_SECTOR == 0) //���� ��� �� ��ġ�� ù ��° ���Ͷ��
			{
				//�ش� ����� �Ϲ� ���ȭ �� ��, ���� �� ���, �� ���� ���� ����
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = true;
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;

				if (Flash_write(flashmem, &meta_buffer, PSN, src_data) == COMPLETE)
					goto OVERWRITE_ERR;
			}
			else
			{
				if (Flash_write(flashmem, &meta_buffer, PSN, src_data) == COMPLETE)
					goto OVERWRITE_ERR;
			}

			delete meta_buffer;
			meta_buffer = NULL;
		}
		else //����ִ� ��ġ
		{
			//do nothing
		}
	}
	if (block_read_buffer[0] == NULL) //���� ù ��° ����(������)�� �ش��ϴ� ��� ���� ������ index 0�� ����־ �ش� ��� ������ ������� �ʾ��� ��� ����
	{
		//�ش� ����� �Ϲ� ���ȭ �� ��, ���� �� ��� ���� ����
		PSN = empty_spare_block * BLOCK_PER_SECTOR; //�ش� ����� ù ��° ����(������)�� �ʱ�ȭ
		meta_buffer = SPARE_read(flashmem, PSN);
		meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = true;
		meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
		SPARE_write(flashmem, PSN, &meta_buffer);
		delete meta_buffer;
		meta_buffer = NULL;
	}

	//��� ���� ���̺�� Spare Block ���̺� �󿡼� SWAP
	SWAP((*flashmem)->block_level_mapping_table[LBN], (*flashmem)->spare_block_table->table_array[spare_block_table_index], tmp);

	goto END_SUCCESS;


HYBRID_LOG_DYNAMIC: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	PBN1 = (*flashmem)->log_block_level_mapping_table[LBN][0]; //������ ����� ���� ��� ��ȣ(PBN1)
	PBN2 = (*flashmem)->log_block_level_mapping_table[LBN][1]; //������ ����� ���� ��� ��ȣ(PBN2)
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31

	/***
		�Ϸķ� �� ������ ���̺�(Loffset -> Poffset)���� �ش� ��ġ�� ã���� �Ѵّm,
		(PBN*��ϴ� ���ͼ�) + (LSN % ��ϴ� ���ͼ�)
		�ش� ���� ����� ������ ���̺�󿡼��� ���� ��ġ + ������ ���̺� �������� ��ġ
	***/

	/***
		PBN2�� �Ҵ�Ǿ� �ִ� ���
		---
		��,	PBN1�� ��� ������ ��ü ũ�⿡ ���� ��ϵ� ��� �����Ͱ� ��ȿȭ�Ǿ� ���̺��� Unlinked(����)�� ���
		(Ư�� �����¿� ���� �ݺ��� ���Ⱑ �߻����� �ʰ�, PBN1���� ��� �����¿� ���� Overwrite�� �߻��� ���)
		=> PBN2���� PBN1�κ��� Overwrite�� �����͵��� ��ϵǾ��ִ�.
	***/
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 != DYNAMIC_MAPPING_INIT_VALUE)
	{
		//PBN2�� ���� ��ġ ��ȿȭ
		Loffset = LSN % BLOCK_PER_SECTOR; //������ ���� ���̺� �������� LSN�� �� ������
		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
		Poffset = (*flashmem)->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
		PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

		meta_buffer = SPARE_read(flashmem, PSN);
		meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;
		SPARE_write(flashmem, PSN, &meta_buffer);

		delete meta_buffer;
		meta_buffer = NULL;

		//������ ���̺� ����
		(*flashmem)->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE;

		goto HYBRID_LOG_DYNAMIC_PBN1_PROC;
	}
	/***
		PBN1, PBN2 ��� �Ҵ���� ���� ���
		PBN1�� �Ҵ�Ǿ� �ִ� ���
		PBN1, PBN2 ��� �Ҵ�� ���
		---
		=> PBN1�� ���� ���� ����
	***/
	else
		goto HYBRID_LOG_DYNAMIC_PBN1_PROC;

HYBRID_LOG_DYNAMIC_PBN1_PROC:
	/*** ����, PBN1�� �Ҵ�Ǿ� ���� ���� ���, Spare Block�� �ƴ� �� ����� �Ҵ� ***/
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE)
	{
		if (search_empty_block(flashmem, PBN1, &meta_buffer, mapping_method, table_type) != SUCCESS) //�� �Ϲ� ���(PBN)�� ���������� Ž���Ͽ� PBN ��, �ش� PBN�� meta���� �޾ƿ´�
			goto END_NO_SPACE; //��� ���� ���� ����

		(*flashmem)->log_block_level_mapping_table[LBN][0] = PBN1;
		Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31
		PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

		if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ��� �� ��� ���� ����
		{
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;

			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, &meta_buffer, PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;
			
			delete meta_buffer;
			meta_buffer = NULL;
		}
		else //��� �� ��ġ�� ����� ù ��° ���Ͱ� �ƴϸ� �� ��� ������ ���� ����
		{
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
			SPARE_write(flashmem, (PBN * BLOCK_PER_SECTOR), &meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 
			
			delete meta_buffer;
			meta_buffer = NULL;

			//��� �� ��ġ�� meta������ �д´�
			meta_buffer = SPARE_read(flashmem, PSN);

#if DEBUG_MODE == 1
			//�� ����̹Ƿ�, ��ϵ� �����Ͱ� �������� ����
			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true ||
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true)
				goto WRONG_META_ERR; //�߸��� meta ���� ����
#endif

			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, &meta_buffer, PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;
			
			delete meta_buffer;
			meta_buffer = NULL;
		}

		goto END_SUCCESS; //����
	}
	else //PBN1�� �Ҵ�Ǿ� �ְ�
	{
		Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31
		PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

		//meta������ ���� �Ǻ�
		meta_buffer = SPARE_read(flashmem, PSN);

		/*** ����, PBN1���� ��ϵ��� ���� �� ��ġ�� ��� ***/
		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
		{

			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, &meta_buffer, PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;
			
			delete meta_buffer;
			meta_buffer = NULL;

			/***
				PBN2������ �� ������ ��ġ�� ������ PBN1���� Overwrite�Ǿ� PBN2���� ���ǰ� �ִ� ��ġ�� ���
				PBN1�� PBN2�� ���� �����Ϳ� ���Ͽ� 1ȸ�� Overwrite�� ������ Log Blockó�� ����Ѵ�.
			***/
			if (PBN2 != DYNAMIC_MAPPING_INIT_VALUE) //PBN2�� �Ҵ�Ǿ������� ���� ������ ��ȿȭ
			{
				offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
				Poffset = (*flashmem)->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
				PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset; //PBN1�� ���� �� �������� PBN2�� ���� ������ ��ġ

				//meta������ ���� �Ǻ�
				meta_buffer = SPARE_read(flashmem, PSN);

#if DEBUG_MODE == 1  
				//�߸��� meta���� ��������
				if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true ||
					meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true) //��ȿ���� �ʰų� �� ������ ���
					goto WRONG_META_ERR; //�߸��� meta����
#endif
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;
				(*flashmem)->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE;

				SPARE_write(flashmem, PSN, &meta_buffer);
				
				delete meta_buffer;
				meta_buffer = NULL;

				//�̿� ���� ����, PBN2�� ��� �����Ͱ� ��ȿȭ�Ǿ�����, PBN2 ��ȿȭ
				update_victim_block_info(flashmem, false, PBN2, mapping_method);
				if ((*flashmem)->victim_block_info.victim_block_invalid_ratio == 1.0)
				{
					PSN = (PBN2 * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������
					meta_buffer = SPARE_read(flashmem, PSN);
					meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] = false;
					SPARE_write(flashmem, PSN, &meta_buffer);
					
					delete meta_buffer;
					meta_buffer = NULL;

					//PBN2�� ��� ���� ���� ���̺� �󿡼� Unlink(���� ����)
					(*flashmem)->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE;
				}
				else //GC�� ���� Victim Block���� �������� �ʵ��� ����ü �ʱ�ȭ
					(*flashmem)->victim_block_info.clear_all();
			}

			goto END_SUCCESS; //����
		}
		/*** ����, PBN1���� �̹� ��ϵǾ��ְ� ��ȿ�� �������� ��� ***/
		else if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
		{
			//�ش� ��ġ ��ȿȭ
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;
			SPARE_write(flashmem, PSN, &meta_buffer);
			
			delete meta_buffer;
			meta_buffer = NULL;

			//�̿� ���� ����, PBN1�� ��� �����Ͱ� ��ȿȭ�Ǿ�����, PBN1 ��ȿȭ
			update_victim_block_info(flashmem, false, PBN1, mapping_method);
			if ((*flashmem)->victim_block_info.victim_block_invalid_ratio == 1.0)
			{
				PSN = (PBN1 * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������
				meta_buffer = SPARE_read(flashmem, PSN);
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] = false;
				SPARE_write(flashmem, PSN, &meta_buffer);
				
				delete meta_buffer;
				meta_buffer = NULL;

				//PBN1�� ��� ���� ���� ���̺� �󿡼� Unlink(���� ����)
				(*flashmem)->log_block_level_mapping_table[LBN][0] = DYNAMIC_MAPPING_INIT_VALUE;
			}
			else //GC�� ���� Victim Block���� �������� �ʵ��� ����ü �ʱ�ȭ
				(*flashmem)->victim_block_info.clear_all();

			goto HYBRID_LOG_DYNAMIC_PBN2_PROC; //PBN2�� ���� ���� ����
		}
		/*** ����, PBN1���� �̹� ��ϵǾ��ְ� ��ȿ�� �������� ��� ***/
		else if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == false)
		{
			delete meta_buffer;
			meta_buffer = NULL;

			goto HYBRID_LOG_DYNAMIC_PBN2_PROC; //PBN2�� ���� ���� ����
		}
		else //���� : ��ϵǾ� ���� �ʰ�, ��ȿ�� ������
			goto WRONG_META_ERR;
	}

HYBRID_LOG_DYNAMIC_PBN2_PROC:
	/*** ����, PBN2�� �Ҵ�Ǿ� ���� ���� ���, Spare Block�� �ƴ� �� ����� �Ҵ� ***/
	if (PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
	{
		if (search_empty_block(flashmem, PBN2, &meta_buffer, mapping_method, table_type) != SUCCESS) //�� �Ϲ� ���(PBN)�� ���������� Ž���Ͽ� PBN ��, �ش� PBN�� meta���� �޾ƿ´�
			goto END_NO_SPACE; //��� ���� ���� ����

		(*flashmem)->log_block_level_mapping_table[LBN][1] = PBN2;

		/*** 
			���������� ����ִ� ������ ��ġ�� ��� �Ѵ� 
			=> ó�� �Ҵ�Ǿ����Ƿ� 0�� �����¿� ���
		***/

		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
		Poffset = (*flashmem)->offset_level_mapping_table[offset_level_table_index] = 0; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
		PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

		meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;

		//�ش� ������ ��ġ�� ���
		if (Flash_write(flashmem, &meta_buffer, PSN, src_data) == COMPLETE)
			goto OVERWRITE_ERR;
		
		delete meta_buffer;
		meta_buffer = NULL;

		goto END_SUCCESS; //����
	}
	else //PBN2�� �Ҵ�Ǿ� �ְ�
	{
		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
		Poffset = (*flashmem)->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ

		/*** PBN2������ �� ������ ��ġ�� PBN1���� Overwrite�Ǿ� ���ǰ� �ִ� ��ġ�� ��� ***/
		if (Poffset != OFFSET_MAPPING_INIT_VALUE)
		{
			PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset; //��� �� ���� ��ġ
			meta_buffer = SPARE_read(flashmem, PSN);
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;
			SPARE_write(flashmem, PSN, &meta_buffer);
			
			delete meta_buffer;
			meta_buffer = NULL;
		}

		if (search_empty_offset_in_block(flashmem, PBN2, Poffset, &meta_buffer) != SUCCESS)
		{
			/*** ����, PBN2�� ��� ������ �� ������ ���� ��� ***/
			/*** PBN1�� PBN2�� ������ Spare Block�� ����Ͽ� ��ȿ ������ Merge ���� ***/
			if (full_merge(flashmem, LBN, mapping_method) != SUCCESS)
				goto WRONG_META_ERR;
			else
			{
				delete meta_buffer;
				meta_buffer = NULL;

				goto HYBRID_LOG_DYNAMIC; //Merge ����� PBN1�� ���Ͽ� �� ����
			}
		}

		/*** ����, PBN2�� ��� ������ �� ���� ���� �� ***/
		(*flashmem)->offset_level_mapping_table[offset_level_table_index] = Poffset;
		PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

		//�ش� ������ ��ġ�� ���
		if (Flash_write(flashmem, &meta_buffer, PSN, src_data) == COMPLETE)
			goto OVERWRITE_ERR;

		delete meta_buffer;
		meta_buffer = NULL;

		goto END_SUCCESS; //����
	}

END_SUCCESS: //���� ����
	if (meta_buffer != NULL)
		goto MEM_LEAK_ERR;

	(*flashmem)->save_table(mapping_method, table_type);
	(*flashmem)->gc->scheduler(flashmem, mapping_method);

	return SUCCESS;

END_NO_SPACE: //��� ���� ���� ����
	fprintf(stderr, "��� �� ������ �������� ���� : �Ҵ�� ũ���� ���� ��� ���\n");
	return FAIL;

WRONG_META_ERR: //�߸��� meta���� ����
	fprintf(stderr, "���� : �߸��� meta ����\n");
	system("pause");
	exit(1);

WRONG_TABLE_TYPE_ERR: //�߸��� ���̺� Ÿ��
	fprintf(stderr, "���� : �߸��� ���̺� Ÿ��\n");
	system("pause");
	exit(1);

WRONG_STATIC_TABLE_ERR: //�߸��� ���� ���̺� ����
	fprintf(stderr, "���� : ���� ���̺� ���� �������� ���� ���̺�\n");
	system("pause");
	exit(1);

OVERWRITE_ERR: //Overwrite ����
	fprintf(stderr, "���� : Overwrite�� ���� ���� �߻�\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "���� : meta_buffer�� ���� �޸� ���� �߻�\n");
	system("pause");
	exit(1);
}

int full_merge(FlashMem** flashmem, unsigned int LBN, int mapping_method) //Ư�� LBN�� ������ PBN1�� PBN2�� ���Ͽ� Merge ����
{
	unsigned int PBN1 = (*flashmem)->log_block_level_mapping_table[LBN][0]; //LBN�� ������ ���� ���(PBN1)
	unsigned int PBN2 = (*flashmem)->log_block_level_mapping_table[LBN][1]; //LBN�� ������ ���� ���(PBN2)
	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ

	__int8 Loffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ���� ���� Offset = 0 ~ 31
	__int8 Poffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ������ ���� Offset = 0 ~ 31
	unsigned int offset_level_table_index = DYNAMIC_MAPPING_INIT_VALUE; //������ ���� ���̺� �ε���

	unsigned int empty_spare_block = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare ���
	unsigned int spare_block_table_index = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare �����  Spare ��� ���̺� �� �ε���
	unsigned int tmp = DYNAMIC_MAPPING_INIT_VALUE; //���̺� SWAP���� �ӽ� ����

	char read_buffer = NULL; //���� ���ͷκ��� �о���� ������
	char block_read_buffer[BLOCK_PER_SECTOR] = { NULL }; //�� ��ϳ��� ������ �ӽ� ���� ����
	__int8 read_buffer_index = 0; //�����͸� �о �ӽ������ϱ� ���� read_buffer �ε��� ����

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	switch (mapping_method) //���� ����
	{
	case 3:
		break;

	default:
		return FAIL;
	}

	f_flash_info = (*flashmem)->get_f_flash_info();

	/***
		Merge ���� :
		- PBN1�� PBN2�� ��ȿ�� ��(�ʱⰪ�� �ƴ� ������ ��)�̾�� ��
		- PBN1�� PBN2�� Spare ����� �ƴ� ��
		========================================================================
		��� �����Ϳ� ���� ����� �� �� ���� PBN1�� ��ϵǰ�, Overwrite�ÿ� PBN2�� ��ϵǹǷ�
		- PBN1���� � �� �������� ��ȿ�� ���̶�� PBN2���� �ش� �� �����¿� ��ȿ�� �����Ͱ� ����
		- PBN1���� � �� �����¿� ���� ��ȿ�� ���̶�� PBN2���� �ش� �� �����¿� ���� ��ϵǾ� ���� ���� (�׻�)
		= PBN1���� � �� �����¿� ���� ��ȿ�� ���̶�� PBN2���� �ش� �� �����¿� ���� ��ȿ�� ���������� �ʰ� �׻� ��ϵǾ� ���� ����
		=> Overwrite�ÿ� PBN2�� ����ϹǷ�, PBN1�� � �� �������� ��ȿ�ϴٸ� PBN2�� ���� �� �������� ������� �� �ۿ� ����
		========================================================================
		PBN1�� PBN2�κ��� �� �����¿� ���� ��ȿ �����͵��� ���۷� ����
		PBN1�� �� ������ ��ġ�� ��ȿ�� ��� PBN2���� �ش� �� ������ ��ġ�� �д´�
		=> PBN1�� �� �Ҵ���
	***/

	//Merge�� ���� PBN1�� PBN2�� ���� �� �����Ǿ� �־�� ��
	if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 != DYNAMIC_MAPPING_INIT_VALUE)
	{
#if DEBUG_MODE == 1
		/***
			�� ����� ù ��° �������� �о� Spare ������� �Ǻ�(���� ����)
			�Ϲ� ��� ���� ���̺�κ��� �ε����Ͽ� ��� ��ȣ(LBN => PBN1, PBN2)�� �����Ƿ�, �׻� Spare ����� �ƴ�����, meta ���� ���� ������ ���Ͽ� �˻��Ѵ�
		***/
		PSN = (PBN1 * BLOCK_PER_SECTOR);
		meta_buffer = SPARE_read(flashmem, PSN);

		//PBN1�� Spare ����� �ƴ� ��쿡
		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] == true)
		{
			delete meta_buffer;

			PSN = (PBN2 * BLOCK_PER_SECTOR);
			meta_buffer = SPARE_read(flashmem, PSN);
			//PBN2�� Spare ����� �ƴ� ��쿡
			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] == true)
				delete meta_buffer;
			else //Spare ����� ���
				goto WRONG_META_ERR;

		}
		else //Spare ����� ���
			goto WRONG_META_ERR;
#endif
	}
	else
		return COMPLETE; //PBN1, PBN2 �� �� �����Ǿ� ���� ������, �������� �ʴ´�.

	/***
		PBN1�� PBN2�κ��� �� �����¿� ���� ��ȿ �����͵��� ���۷� ����
		1) PBN1�� �� ������ ��ġ�� ��ȿ�� ���
		2) PBN2���� �ش� �� ������ ��ġ�� �д´�
	***/
	for (Loffset = 0; Loffset < BLOCK_PER_SECTOR; Loffset++)
	{
		PSN = (PBN1 * BLOCK_PER_SECTOR) + Loffset; //PBN1 �ε���
		meta_buffer = SPARE_read(flashmem, PSN);

		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
		{
			//PBN1���� ������� �ʰ�, ��ȿ�ϸ�
			Flash_read(flashmem, NULL, PSN, block_read_buffer[Loffset]);
		}

		else if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true &&
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
		{
			//PBN1���� ����ְ�, ��ȿ�ϸ� �������� ���߱����� �� �������� ��� (��, �ѹ��� ��ϵ��� ���� ��ġ)
			block_read_buffer[Loffset] = NULL;
		}
		else //PBN1���� ��ȿ���� ������
		{
			//PBN2���� �ش� �� ������ ��ġ�� �д´�
			offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //PBN2�� ������ ���� ���̺� �������� PBN1������ Loffset�� �ش��ϴ� index��
			Poffset = (*flashmem)->offset_level_mapping_table[offset_level_table_index]; //���� ��� ���� ������ ������ ��ġ
			PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset; //������ ����� ���� ���� ��ȣ

			Flash_read(flashmem, NULL, PSN, block_read_buffer[Loffset]);
		}

		delete meta_buffer;
	}

	/*** PBN1, PBN2 Erase�� �ϳ���(PBN1) Spare ������� ���� ***/
	Flash_erase(flashmem, PBN1); //PBN1�� erase
	Flash_erase(flashmem, PBN2); //PBN2�� erase
	PSN = PBN1 * BLOCK_PER_SECTOR; //�ش� ����� ù ��° ����(������)
	meta_buffer = SPARE_read(flashmem, PSN);
	meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = false;
	SPARE_write(flashmem, PSN, &meta_buffer);
	delete meta_buffer;

	/*** �� Spare ����� ã�Ƽ� ��� ***/
	(*flashmem)->spare_block_table->rr_read(empty_spare_block, spare_block_table_index);

	for (Loffset = 0; Loffset < BLOCK_PER_SECTOR; Loffset++)
	{
		PSN = (empty_spare_block * BLOCK_PER_SECTOR) + Loffset; //�����͸� �ű� Spare ��� �ε���

		if (block_read_buffer[Loffset] != NULL) //��� ������ �о���� ���ۿ��� �ش� ��ġ�� ������� ������, �� ��ȿ�� �������̸�
		{
			meta_buffer = SPARE_read(flashmem, PSN); //����(������)�� Spare ���� �б�

			if (PSN % BLOCK_PER_SECTOR == 0) //���� ��� �� ��ġ�� ù ��° ���Ͷ��
			{
				//�ش� ����� �Ϲ� ���ȭ �� ��, ���� �� ���, �� ���� ���� ����
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = true;
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
				if (Flash_write(flashmem, &meta_buffer, PSN, block_read_buffer[Loffset]) == COMPLETE)
					goto OVERWRITE_ERR;
			}
			else
			{
				if (Flash_write(flashmem, &meta_buffer, PSN, block_read_buffer[Loffset]) == COMPLETE)
					goto OVERWRITE_ERR;
			}

			delete meta_buffer;
		}
		//��ȿ�ϰų� ������� ��� ������� �ʴ´� 
	}
	if (block_read_buffer[0] == NULL) //���� ù ��° ����(������)�� �ش��ϴ� ��� ���� ������ index 0�� ����־ �ش� ��� ������ ������� �ʾ��� ��� ����
	{
		//�ش� ����� �Ϲ� ���ȭ �� ��, ���� �� ��� ���� ����
		PSN = empty_spare_block * BLOCK_PER_SECTOR; //�ش� ����� ù ��° ����(������)�� �ʱ�ȭ
		meta_buffer = SPARE_read(flashmem, PSN);
		meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = true;
		meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
		SPARE_write(flashmem, PSN, &meta_buffer);
		delete meta_buffer;
	}

	/*** PBN2 ��� ���� ���̺�, ������ ���� ���̺� �ʱ�ȭ ***/
	for (Loffset = 0; Loffset < BLOCK_PER_SECTOR; Loffset++)
	{
		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� index��
		(*flashmem)->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE;
	}
	PBN2 = (*flashmem)->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE;

	/*** PBN1�� Spare ��� ���̺� ���� ***/
	//PBN1�� �������� �׻� ��ġ��Ű���� �Ͽ����Ƿ�, ��� ���� ���̺� ����
	SWAP((*flashmem)->log_block_level_mapping_table[LBN][0], (*flashmem)->spare_block_table->table_array[spare_block_table_index], tmp); //PBN1�� Spare ��� ��ü

	(*flashmem)->save_table(mapping_method, table_type);
	return SUCCESS;


WRONG_META_ERR: //�߸��� meta���� ����
	fprintf(stderr, "���� : �߸��� meta ����\n");
	system("pause");
	exit(1);

OVERWRITE_ERR: //Overwrite ����
	fprintf(stderr, "���� : Overwrite�� ���� ���� �߻�\n");
	system("pause");
	exit(1);
}

int full_merge(FlashMem** flashmem, int mapping_method) //���̺��� �����Ǵ� ��� ��Ͽ� ���� Merge ����
{
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	switch (mapping_method) //���� ����
	{
	case 3:
		break;

	default:
		return FAIL;
	}

	f_flash_info = (*flashmem)->get_f_flash_info();

	for (unsigned int LBN = 0; LBN < (f_flash_info.block_size - f_flash_info.spare_block_size); LBN++)
	{
		//�Ϲ� ��� ���� ���� ���̺��� �����Ǵ� ��� �Ϲ� ��Ͽ� ���� ������ ��� Merge ����
		if (full_merge(flashmem, LBN, mapping_method) == FAIL)
			return FAIL;
	}

	return SUCCESS;
}

int trace(FlashMem** flashmem, int mapping_method, int table_type) //Ư�� ���Ͽ� ���� ���� ������ �����ϴ� �Լ�
{
	//��ü trace �� �����ϱ� ���� ����� read, write, erase Ƚ�� ���

	FILE* trace_file = NULL; //trace ���� �Է� ����
	
#if BLOCK_TRACE_MODE == 1 //Trace for Per Block Wear-leveling
	FILE* trace_per_block_output = NULL; //��� �� trace ��� ���
	F_FLASH_INFO f_flash_info;
	V_FLASH_INFO* block_trace_array = NULL; //��ü ��Ͽ� ���� �� ��� �� ���� trace ���� �迭 (index : PBN)
	
	f_flash_info = (*flashmem)->get_f_flash_info();
	block_trace_array = new V_FLASH_INFO[f_flash_info.block_size]; //��ü ��� ���� ũ��� �Ҵ�
	for(unsigned int i=0; i < f_flash_info.block_size; i++)
		block_trace_array[i].clear_trace_info(); //�б�, ����, ����� Ƚ�� �ʱ�ȭ
#endif

	char file_name[100]; //trace ���� �̸�
	char op_code[2] = { 0, }; //�����ڵ�(w,r,e) : '\0' ���� 2�ڸ�
	unsigned int LSN = UINT32_MAX; //LSN
	char dummy_data = 'A'; //trace�� ���� ���� ������
	
	system("cls");
	std::cout << "< ���� ���� ��� >" << std::endl;
	system("dir");
	std::cout << "trace ���� �̸� �Է� (�̸�.Ȯ����) >>";
	gets_s(file_name, 100);

	trace_file = fopen(file_name, "rt"); //�б� + �ؽ�Ʈ ���� ���

	if (trace_file == NULL)
	{
		fprintf(stderr, "File does not exist or cannot be opened.\n");
		return FAIL;
	}

	std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now(); //trace �ð� ���� ����
	while (!feof(trace_file))
	{
		memset(op_code, NULL, sizeof(op_code)); //�����ڵ� ���� �ʱ�ȭ
		LSN = UINT32_MAX; //���� ������ �÷��� �޸��� �뷮���� ���� �� ���� ���� LSN ������ �ʱ�ȭ

		fscanf(trace_file, "%s\t%u\n", &op_code, &LSN); //������ �и��� ���� �б�
		if (strcmp(op_code, "w") == 0 || strcmp(op_code, "W") == 0)
		{
			FTL_write(flashmem, LSN, dummy_data, mapping_method, table_type);
		}
	}
	fclose(trace_file);
	std::chrono::system_clock::time_point end_time = std::chrono::system_clock::now(); //trace �ð� ���� ��
	std::chrono::milliseconds mill_end_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << ">> Trace function ends : " << mill_end_time.count() <<"milliseconds elapsed"<< std::endl;

#if BLOCK_TRACE_MODE == 1 //Trace for Per Block Wear-leveling
	//��� �� ����
	trace_per_block_output = fopen("trace_per_block_result.txt", "wt");
	if (trace_per_block_output == NULL)
	{
		fprintf(stderr, "��� �� ���� ��� ���� ���� (trace_per_block_result.txt)\n");
		return FAIL;
	}
	for (unsigned int PBN = 0; PBN < f_flash_info.block_size; PBN++)
	{
		//PBN [TAB] �б� Ƚ�� [TAB] ���� Ƚ�� [TAB] ����� Ƚ�� ���
		fprintf(trace_per_block_output, "%u\t%d\t%d\t%d\n", PBN, block_trace_array[PBN].flash_read_count, block_trace_array[PBN].flash_write_count, block_trace_array[PBN].flash_erase_count);
	}
	delete[] block_trace_array;
	printf(">> ��� �� ���� ���� trace_per_block_result.txt�� ��ϵ�(PBN [TAB] �б� Ƚ�� [TAB] ���� Ƚ�� [TAB] ����� Ƚ��)\n");
#endif

	return SUCCESS;
}