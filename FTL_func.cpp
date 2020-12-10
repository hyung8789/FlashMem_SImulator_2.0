#include "FlashMem.h"

// Print_table, FTL_read, FTL_write, trace ����
// �� ���� ��ȣ �Ǵ� �� ��� ��ȣ�� �������̺�� �����Ͽ� physical_func���� read, write, erase�� �����Ͽ� �۾��� ����

int Print_table(FlashMem*& flashmem, MAPPING_METHOD mapping_method, TABLE_TYPE table_type)  //���� ���̺� ���
{
	FILE* table_output = NULL; //���̺��� ���Ϸ� ����ϱ� ���� ���� ������

	unsigned int block_table_size = 0; //��� ���� ���̺��� ũ��
	unsigned int spare_table_size = 0; //Spare block ���̺��� ũ��
	unsigned int offset_table_size = 0; //������ ���� ���̺��� ũ��

	unsigned int table_index = 0; //���� ���̺� �ε���

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�


	if ((table_output = fopen("table.txt", "wt")) == NULL)
	{
		fprintf(stderr, "table.txt ������ ������� �� �� �����ϴ�. (Print_table)\n");

		return FAIL;
	}

	switch (mapping_method) //���� ��Ŀ� ���� ���� ���̺� ��� ����
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		block_table_size = f_flash_info.block_size - f_flash_info.spare_block_size; //�Ϲ� ��� ��
		spare_table_size = f_flash_info.spare_block_size; //Spare block ��

		std::cout << "< Block Mapping Table (LBN -> PBN) >" << std::endl;
		fprintf(table_output, "< Block Mapping Table (LBN -> PBN) >\n");

		while (table_index < block_table_size)
		{
			//�Ҵ�� ���鸸 ���
			if (flashmem->block_level_mapping_table[table_index] == DYNAMIC_MAPPING_INIT_VALUE)
			{
				table_index++;
			}
			else
			{
				printf("%u -> %u\n", table_index, flashmem->block_level_mapping_table[table_index]);
				fprintf(table_output, "%u -> %u\n", table_index, flashmem->block_level_mapping_table[table_index]);

				table_index++;
			}
		}

		std::cout << "\n< Spare Block Table (Index -> PBN) >" << std::endl;
		fprintf(table_output, "\n< Spare Block Table (Index -> PBN) >\n");
		table_index = 0;
		while (table_index < spare_table_size)
		{
			printf("%u -> %u\n", table_index, flashmem->spare_block_table->table_array[table_index]);
			fprintf(table_output, "%u -> %u\n", table_index, flashmem->spare_block_table->table_array[table_index]);

			table_index++;
		}

		break;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		block_table_size = f_flash_info.block_size - f_flash_info.spare_block_size; //�Ϲ� ��� ��
		spare_table_size = f_flash_info.spare_block_size; //Spare block ��
		offset_table_size = f_flash_info.block_size * BLOCK_PER_SECTOR; //������ ���̺��� ũ��

		std::cout << "< Hybrid Mapping Block level Table (LBN -> PBN1, PBN2) >" << std::endl;
		fprintf(table_output, "< Hybrid Mapping Block level Table (LBN -> PBN1, PBN2) >\n");

		while (table_index < block_table_size)
		{
			//�Ҵ�� ���鸸 ���
			if (flashmem->log_block_level_mapping_table[table_index][0] == DYNAMIC_MAPPING_INIT_VALUE &&
				flashmem->log_block_level_mapping_table[table_index][1] == DYNAMIC_MAPPING_INIT_VALUE) //PBN1�� PBN2�� �Ҵ���� ���� ���
			{
				table_index++;
			}
			else if (flashmem->log_block_level_mapping_table[table_index][0] != DYNAMIC_MAPPING_INIT_VALUE &&
				flashmem->log_block_level_mapping_table[table_index][1] == DYNAMIC_MAPPING_INIT_VALUE) //PBN1 �Ҵ� PBN2�� �Ҵ���� ���� ���
			{
				printf("%u -> %u, non-assigned\n", table_index, flashmem->log_block_level_mapping_table[table_index][0]);
				fprintf(table_output, "%u -> %u, non-assigned\n", table_index, flashmem->log_block_level_mapping_table[table_index][0]);

				table_index++;
			}
			else if (flashmem->log_block_level_mapping_table[table_index][0] == DYNAMIC_MAPPING_INIT_VALUE &&
				flashmem->log_block_level_mapping_table[table_index][1] != DYNAMIC_MAPPING_INIT_VALUE) //PBN1�� �Ҵ���� �ʰ� PBN2�� �Ҵ� �� ���
			{
				printf("%u -> non-assigned, %u\n", table_index, flashmem->log_block_level_mapping_table[table_index][1]);
				fprintf(table_output, "%u -> non-assigned, %u\n", table_index, flashmem->log_block_level_mapping_table[table_index][1]);

				table_index++;
			}
			else //PBN1, PBN2 ��� �Ҵ� �� ���
			{
				printf("%u -> %u, %u\n", table_index, flashmem->log_block_level_mapping_table[table_index][0], flashmem->log_block_level_mapping_table[table_index][1]);
				fprintf(table_output, "%u -> %u, %u\n", table_index, flashmem->log_block_level_mapping_table[table_index][0], flashmem->log_block_level_mapping_table[table_index][1]);

				table_index++;
			}
		}

		std::cout << "\n< Spare Block Table (Index -> PBN) >" << std::endl;
		fprintf(table_output, "\n< Spare Block Table (Index -> PBN) >\n");
		table_index = 0;
		while (table_index < spare_table_size)
		{
			printf("%u -> %u\n", table_index, flashmem->spare_block_table->table_array[table_index]);
			fprintf(table_output, "%u -> %u\n", table_index, flashmem->spare_block_table->table_array[table_index]);

			table_index++;
		}

		std::cout << "\n< Hybrid Mapping Offset level Table (Index -> POffset) >" << std::endl;
		fprintf(table_output, "\n< Hybrid Mapping Offset level Table (Index -> POffset) >\n");
		table_index = 0;
		while (table_index < offset_table_size)
		{
			//�Ҵ�� ���鸸 ���
			if (flashmem->offset_level_mapping_table[table_index] == OFFSET_MAPPING_INIT_VALUE)
			{
				table_index++;
			}
			else
			{
				printf("%u -> %d\n", table_index, flashmem->offset_level_mapping_table[table_index]);
				fprintf(table_output, "%u -> %d\n", table_index, flashmem->offset_level_mapping_table[table_index]);
 				table_index++;
			}
		}
		break;

	default:
		fclose(table_output);
		return FAIL;
	}

	fclose(table_output);
	std::cout << ">> table.txt" << std::endl;
	system("notepad table.txt");

	return SUCCESS;
}

int FTL_read(FlashMem*& flashmem, unsigned int LSN, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //////�� ���� �Ǵ� �� ��Ͽ� �ش�Ǵ� �������̺� �� ���� ���� �Ǵ� ���� ����� ��ġ�� ��ȯ
{
	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE; //�ش� ���� ���Ͱ� ��ġ�ϰ� �ִ� ���� ���
	unsigned int PBN1 = DYNAMIC_MAPPING_INIT_VALUE; //������ ���
	unsigned int PBN2 = DYNAMIC_MAPPING_INIT_VALUE; //�α� ���
	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ
	unsigned int offset_level_table_index = DYNAMIC_MAPPING_INIT_VALUE; //������ ���� ���̺� �ε���

	__int8 Loffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ���� ���� Offset = 0 ~ 31
	__int8 Poffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ������ ���� Offset = 0 ~ 31

	char read_buffer = NULL; //���� ���ͷκ��� �о���� ������

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	//�ý��ۿ��� ����ϴ� Spare Block�� ����(������)����ŭ ����
	if (LSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - (f_flash_info.spare_block_size * BLOCK_PER_SECTOR) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	/*
	
	�ߺ��Ǵ� ó�� �ڵ忡 ���� �����丵 ����
	���̺긮�� �α� �˰��� ����
	PBN1�� PBN2�� ���� ���� ���̺� �Ҵ�Ǿ� �ִ� ��Ȳ�� ���� �� ó�� ��ƾ���� �б�
	
	1	PBN1�� ���� �б� ó�� ��ƾ
	2	PBN2�� ���� �б� ó�� ��ƾ

	����� ��尡 �ƴ� �� PBN2�� ������ ���� ���̺��� �Ҵ� ���¸� ���Ͽ� �ٷ� �Ǻ� ����
	
	����, PBN1�� PBN2 ��� �Ҵ� �Ǿ� ���� �� PBN2���� �ǵ�
	PBN2�� 

	*/
	switch (mapping_method)
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
		PBN = flashmem->block_level_mapping_table[LBN];
		Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
		PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //������ ����� ���� ���� ��ȣ

		if (PBN == DYNAMIC_MAPPING_INIT_VALUE) //Dynamic Table �ʱⰪ�� ���
			goto NON_ASSIGNED_LBN;
		
		SPARE_read(flashmem, PSN, meta_buffer); //Spare ������ ����

#if DEBUG_MODE == 1
		/*** ��ȿȭ�� ��� �Ǻ� - GC�� ���� ���� ó������ �ʾ��� ��� ���� �׽�Ʈ : �Ϲ� ��� ���� ���̺� �����Ǿ� �־�� �ȵ� ***/
		if (PSN % BLOCK_PER_SECTOR == 0) //���� ��ġ�� ����� ù ��° ������ ���
		{
			switch (meta_buffer->block_state)
			{
			case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
				break;

			case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
				goto INVALID_BLOCK_ERR;

			default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
				goto WRONG_ASSIGNED_LBN_ERR;
			}
		}
		else //����� ù ��° ������ Spare ������ ���� �ش� ����� ��ȿȭ �Ǻ�
		{
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			SPARE_read(flashmem, (PBN * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����
			
			switch (meta_buffer->block_state)
			{
			case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;

				SPARE_read(flashmem, PSN, meta_buffer); //���� ��ġ�� Spare ������ ����
				break;

			case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
				goto INVALID_BLOCK_ERR;

			default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
				goto WRONG_ASSIGNED_LBN_ERR;
			}
		}
#endif
		switch (meta_buffer->sector_state) //���� ��ġ�� ���°� ��ȿ �� ��츸 ����
		{
		case SECTOR_STATE::EMPTY:
			goto EMPTY_PAGE;

		case SECTOR_STATE::INVALID:
			goto INVALID_PAGE_ERR;

		case SECTOR_STATE::VALID:
			Flash_read(flashmem, DO_NOT_READ_META_DATA, PSN, read_buffer); //�����͸� �о��

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			goto OUTPUT_DATA_SUCCESS;
		}

		break;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		/***
			PBN1 : �������� �׻� ��ġ��Ŵ
			PBN2 : ������ ���� ���̺��� ���Ͽ� �о����
		***/
		LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
		PBN1 = flashmem->log_block_level_mapping_table[LBN][0]; //PBN1
		PBN2 = flashmem->log_block_level_mapping_table[LBN][1]; //PBN2
		Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
		
		if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //PBN1, PBN2 ��� �Ҵ� ���� ���� ���
			goto NON_ASSIGNED_LBN;
		else if(PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //PBN1�� �Ҵ�� ���
		{ 
			PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset;
			SPARE_read(flashmem, PSN, meta_buffer);
			
#if DEBUG_MODE == 1
			/*** ��ȿȭ�� ��� �Ǻ� - GC�� ���� ���� ó������ �ʾ��� ��� ���� �׽�Ʈ : �Ϲ� ��� ���� ���̺� �����Ǿ� �־�� �ȵ� ***/
			if (PSN % BLOCK_PER_SECTOR == 0) //���� ��ġ�� ����� ù ��° ������ ���
			{
				switch (meta_buffer->block_state)
				{
				case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
					break;

				case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
					goto INVALID_BLOCK_ERR;

				default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
					goto WRONG_ASSIGNED_LBN_ERR;
				}
			}
			else //����� ù ��° ������ Spare ������ ���� �ش� ����� ��ȿȭ �Ǻ�
			{
				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;

				SPARE_read(flashmem, (PBN * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����

				switch (meta_buffer->block_state)
				{
				case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
					if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
						goto MEM_LEAK_ERR;

					SPARE_read(flashmem, PSN, meta_buffer); //���� ��ġ�� Spare ������ ����
					break;

				case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
					goto INVALID_BLOCK_ERR;

				default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
					goto WRONG_ASSIGNED_LBN_ERR;
				}
			}
#endif
			switch (meta_buffer->sector_state) //���� ��ġ�� ���°� ��ȿ �� ��츸 ����
			{
			case SECTOR_STATE::EMPTY:
				goto EMPTY_PAGE;

			case SECTOR_STATE::INVALID:
				goto INVALID_PAGE_ERR;

			case SECTOR_STATE::VALID:
				Flash_read(flashmem, DO_NOT_READ_META_DATA, PSN, read_buffer); //�����͸� �о��

				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;

				goto OUTPUT_DATA_SUCCESS;
			}
		}
		else if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 != DYNAMIC_MAPPING_INIT_VALUE) //PBN2�� �Ҵ�� ���
		{
			Loffset = LSN % BLOCK_PER_SECTOR; //������ ���� ���̺� �������� LSN�� �� ������
			offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
			Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
			PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

#if DEBUG_MODE == 1
			SPARE_read(flashmem, PSN, meta_buffer);
			/*** ��ȿȭ�� ��� �Ǻ� - GC�� ���� ���� ó������ �ʾ��� ��� ���� �׽�Ʈ : �Ϲ� ��� ���� ���̺� �����Ǿ� �־�� �ȵ� ***/
			if (PSN % BLOCK_PER_SECTOR == 0) //���� ��ġ�� ����� ù ��° ������ ���
			{
				switch (meta_buffer->block_state)
				{
				case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
					break;

				case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
					goto INVALID_BLOCK_ERR;

				default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
					goto WRONG_ASSIGNED_LBN_ERR;
				}
			}
			else //����� ù ��° ������ Spare ������ ���� �ش� ����� ��ȿȭ �Ǻ�
			{
				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;

				SPARE_read(flashmem, (PBN2 * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����

				switch (meta_buffer->block_state)
				{
				case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
					if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
						goto MEM_LEAK_ERR;

					SPARE_read(flashmem, PSN, meta_buffer); //���� ��ġ�� Spare ������ ����
					break;

				case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
					goto INVALID_BLOCK_ERR;

				default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
					goto WRONG_ASSIGNED_LBN_ERR;
				}
			}

			switch (meta_buffer->sector_state) //���� ��ġ�� ���°� ��ȿ �� ��츸 ����
			{
			case SECTOR_STATE::EMPTY:
				goto EMPTY_PAGE;

			case SECTOR_STATE::INVALID:
				goto INVALID_PAGE_ERR;

			case SECTOR_STATE::VALID:
				Flash_read(flashmem, DO_NOT_READ_META_DATA, PSN, read_buffer); //�����͸� �о��

				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;

				goto OUTPUT_DATA_SUCCESS;
			}
#endif
			/***
				< ����� ��尡 �ƴ� ��쿡 ���Ͽ� >
				
				PBN2�� ������ ���̺��� �ʱ� �� OFFSET_MAPPING_INIT_VALUE�� �Ҵ�Ǿ��ְ�,
				�׻� ��ȿ�ϰ� ������� ���� ��ġ�� ����Ų��.
				����, ���� ó���� ���Ͽ� �ش� ��ġ�� meta ������ �ǵ����� �ʰ� �ٷ� �����͸� �д´�.
			***/

			if (Poffset != OFFSET_MAPPING_INIT_VALUE)
			{
				Flash_read(flashmem, DO_NOT_READ_META_DATA, PSN, read_buffer); //�����͸� �о��
				goto OUTPUT_DATA_SUCCESS;
			}
			else //������� ��� ������ ���� ���̺��� �������� ����
				goto EMPTY_PAGE;
		
		}
		else //PBN1, PBN2 ��� �Ҵ�� ���
		{
			/*** ���� PBN2�� �д´� ***/
			Loffset = LSN % BLOCK_PER_SECTOR; //������ ���� ���̺� �������� LSN�� �� ������
			offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
			Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
			PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

#if DEBUG_MODE == 1
			SPARE_read(flashmem, PSN, meta_buffer);
			/*** ��ȿȭ�� ��� �Ǻ� - GC�� ���� ���� ó������ �ʾ��� ��� ���� �׽�Ʈ : �Ϲ� ��� ���� ���̺� �����Ǿ� �־�� �ȵ� ***/
			if (PSN % BLOCK_PER_SECTOR == 0) //���� ��ġ�� ����� ù ��° ������ ���
			{
				switch (meta_buffer->block_state)
				{
				case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
					break;

				case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
					goto INVALID_BLOCK_ERR;

				default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
					goto WRONG_ASSIGNED_LBN_ERR;
				}
			}
			else //����� ù ��° ������ Spare ������ ���� �ش� ����� ��ȿȭ �Ǻ�
			{
				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;

				SPARE_read(flashmem, (PBN2 * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����

				switch (meta_buffer->block_state)
				{
				case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
					if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
						goto MEM_LEAK_ERR;

					SPARE_read(flashmem, PSN, meta_buffer); //���� ��ġ�� Spare ������ ����
					break;

				case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
					goto INVALID_BLOCK_ERR;

				default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
					goto WRONG_ASSIGNED_LBN_ERR;
				}
			}

			switch (meta_buffer->sector_state) //���� ��ġ�� ���°� ��ȿ �� ��츸 ����
			{
			case SECTOR_STATE::EMPTY:
				goto EMPTY_PAGE;

			case SECTOR_STATE::INVALID:
				goto INVALID_PAGE_ERR;

			case SECTOR_STATE::VALID:
				Flash_read(flashmem, DO_NOT_READ_META_DATA, PSN, read_buffer); //�����͸� �о��

				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;

				goto OUTPUT_DATA_SUCCESS;
			}
#endif

			/***
				< ����� ��尡 �ƴ� ��쿡 ���Ͽ� >
				
				PBN2�� ������ ���̺��� �ʱ� �� OFFSET_MAPPING_INIT_VALUE�� �Ҵ�Ǿ��ְ�,
				�׻� ��ȿ�ϰ� ������� ���� ��ġ�� ����Ų��.
				����, ���� ó���� ���Ͽ� �ش� ��ġ�� meta ������ �ǵ����� �ʰ� �ٷ� �����͸� �д´�.
			***/

			if (Poffset != OFFSET_MAPPING_INIT_VALUE) //�Ҵ�Ǿ� ���� ��� �׻� ��ȿ�� �������̴�
			{
		
				Flash_read(flashmem, NULL, PSN, read_buffer);
				goto OUTPUT_DATA_SUCCESS;

			}
			else //�Ҵ�Ǿ� ���� ���� ��� PBN1�� �д´�
			{
				Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
				PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset;
				meta_buffer = SPARE_read(flashmem, PSN);
				
			///
			}		
		}
		break;

	default:
		return FAIL;
	}

BLOCK_MAPPING_COMMON_READ_PROC: //��� ���ο� ���� ���� �б� ó�� ��ƾ
HYBRID_LOG_PBN1_PROC: //PBN1�� ó�� ��ƾ
HYBRID_LOG_PBN2_PROC: //PBN2�� ó�� ��ƾ





OUTPUT_DATA_SUCCESS:
	std::cout << "PSN : " << PSN << std::endl; //���� ���� ��ȣ ���
	std::cout << read_buffer << std::endl;

	return SUCCESS;

EMPTY_PAGE: //�� ������
	std::cout << "no data (Empty Page)" << std::endl;

	return COMPLETE;

NON_ASSIGNED_LBN: //���� �Ҵ���� ���� LBN
	std::cout << "no data (Non-Assigned LBN)" << std::endl;

	return COMPLETE;

WRONG_ASSIGNED_LBN_ERR:
	fprintf(stderr, "ġ���� ���� : WRONG_ASSIGNED_LBN_ERR (FTL_read)\n");
	system("pause");
	exit(1);

INVALID_PAGE_ERR:
	fprintf(stderr, "ġ���� ���� : Invalid Page (FTL_read)");
	system("pause");
	exit(1);

INVALID_BLOCK_ERR:
	fprintf(stderr, "ġ���� ���� : Invalid Block (FTL_read)");
	system("pause");
	exit(1);

WRONG_META_ERR:
	fprintf(stderr, "ġ���� ���� : �߸��� meta���� (FTL_read)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (FTL_read)\n");
	system("pause");
	exit(1);
}

int FTL_write(FlashMem*& flashmem, unsigned int LSN, const char src_data, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //�� ���� �Ǵ� �� ��Ͽ� �ش�Ǵ� �������̺� �� ���� ���� �Ǵ� ���� ��� ��ġ�� ���
{
	char block_read_buffer[BLOCK_PER_SECTOR] = { NULL, }; //�� ��� ���� ������ �ӽ� ���� ����
	__int8 read_buffer_index = 0; //�����͸� �о �ӽ������ϱ� ���� read_buffer �ε��� ����
	
	unsigned int empty_spare_block = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare ���
	unsigned int spare_block_table_index = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare �����  Spare ��� ���̺� �� �ε���
	unsigned int tmp = DYNAMIC_MAPPING_INIT_VALUE; //���̺� SWAP���� �ӽ� ����

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

	//Spare area�� ��ϵ� meta-data�� ���� �о���� ����
	META_DATA* meta_buffer = NULL; 
	META_DATA* PBN1_meta_buffer = NULL; //PBN1�� ���� �������� meta ����
	META_DATA* PBN2_meta_buffer = NULL; //PBN2�� ���� �������� meta ����
	META_DATA** PBN1_block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� meta ���� (PBN1)
	META_DATA** PBN2_block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� meta ���� (PBN2)
	bool PBN1_write_proc = false; //PBN1�� ���� ���� �۾� ���� ���� ����
	bool PBN2_write_proc = false; //PBN2�� ���� ���� �۾� ���� ���� ����
	bool is_invalid_block = true; //���� �۾� ���� ���� ����� ��ȿȭ ���� ����

	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	//�ý��ۿ��� ����ϴ� Spare Block�� ����(������)����ŭ ����
	if (LSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - (f_flash_info.spare_block_size * BLOCK_PER_SECTOR) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}

	switch (mapping_method) //���� ��Ŀ� ���� �ش� ó�� ��ġ�� �̵�
	{
	case MAPPING_METHOD::BLOCK: //��� ����

		switch (table_type)
		{
		case TABLE_TYPE::STATIC: //��� ���� Static Table
			goto BLOCK_MAPPING_STATIC;

		case TABLE_TYPE::DYNAMIC: //��� ���� Dynamic Table
			goto BLOCK_MAPPING_DYNAMIC;

		default:
			goto WRONG_TABLE_TYPE_ERR;
		}

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
		goto HYBRID_LOG_DYNAMIC;

	default:
		goto WRONG_MAPPING_METHOD_ERR;
	}

BLOCK_MAPPING_STATIC: //��� ���� Static Table
	//����ڰ� �Է��� LSN���� LBN�� ���ϰ� �����Ǵ� PBN�� ���� ���� ��ȣ�� ����
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
	PBN = flashmem->block_level_mapping_table[LBN]; //������ ����� ���� ��� ��ȣ

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
		if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
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
				
				if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;

				//�ش� ������ ��ġ�� ���
				goto BLOCK_MAPPING_COMMON_WRITE_PROC;
			}
		}
		/*** �ش� ����� ������� ���� ��� ***/
		else
		{
			if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

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
	PBN = flashmem->block_level_mapping_table[LBN]; //������ ����� ���� ��� ��ȣ

	/*** LBN�� PBN�� �������� ���� ��� ***/
	if (PBN == DYNAMIC_MAPPING_INIT_VALUE)
	{
		if (search_empty_normal_block(flashmem, PBN, &meta_buffer, mapping_method, table_type) != SUCCESS) //�� �Ϲ� ���(PBN)�� ���������� Ž���Ͽ� PBN ��, �ش� PBN�� meta���� �޾ƿ´�
			goto END_NO_SPACE; //��� ���� ���� ����

		flashmem->block_level_mapping_table[LBN] = PBN;

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

			if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			//�ش� ������ ��ġ�� ���
			goto BLOCK_MAPPING_COMMON_WRITE_PROC;
		}
	}

	/*** ����� ù ��° �������� Spare������ ���� �ش� ��� ���� �Ǻ� �� ���� ���� ***/
	PSN = (PBN * BLOCK_PER_SECTOR); //�ش� ����� ù ��° ������

#if DEBUG_MODE == 1
	meta_buffer = SPARE_read(flashmem, PSN); //Spare ������ ����

	/*** 
		�ش� ����� ��ȿȭ�� ���, �ش� ����� ����ִ� ���
		---
		Dynamic Table�� ��� �Ϲ� ��� ���� ���̺� �׻� ��ȿȭ�������� ��ϸ� ������Ų��. 
		�̿� ����, ��ȿȭ���� ���� ����� �����Ǵ� ���� �߻����� �ʴ´�.
		����� �����Ǵ� ������, �ش� ��Ͽ� ���� �۾��� �߻��ϹǷ�, �� ����� �̸� �����Ǿ� �ִ� ���� �߻����� �ʴ´�.
	***/
	if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] == false ||
		meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] == true)
		goto WRONG_META_ERR; //�߸��� meta ����

	if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;
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
	
	if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;

	goto END_SUCCESS; //����

BLOCK_MAPPING_COMMON_OVERWRITE_PROC: //��� ���� ���� ó�� ��ƾ 2 : ���ǰ� �ִ� ����� ���� ��ġ�� ���� Overwrite ����
	/***
		1) �ش� PBN�� ��ȿ�� ������(���ο� �����Ͱ� ��ϵ� ��ġ ����) �� ���ο� �����͸� ������ �� Spare Block�� ����Ͽ� 
		��ȿ ������(���ο� �����Ͱ� ��ϵ� ��ġ ����) copy �� ���ο� ������ ���
		2) ���� PBN ���� ��� ���� �� �ش� ��� ��ȿȭ
		3) ���� PBN�� ���� Spare Block ���̺� �� ��ü �� ���� PBN�� Victim Block���� ����
	***/

	if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;

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
			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true)
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;

			SPARE_write(flashmem, PSN, &meta_buffer);
		}
		else
		{
			//����ִ� ���Ͱ� �ƴϸ� �ش� ���� ��ȿȭ
			if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true)
			{
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;
				SPARE_write(flashmem, PSN, &meta_buffer);
			}
			//��������� �ƹ��͵� ���� �ʴ´�.
		}

		if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
	}

	//��ȿȭ�� PBN�� Victim Block���� ���� ���� ���� ���� �� GC �����ٷ� ����
	if (update_victim_block_info(flashmem, false, PBN, mapping_method) != SUCCESS)
		goto VICTIM_BLOCK_INFO_EXCEPTION_ERR;

	/*** �� Spare ����� ã�Ƽ� ��� ***/
	if (flashmem->spare_block_table->rr_read(flashmem, empty_spare_block, spare_block_table_index) == FAIL)
		goto SPARE_BLOCK_EXCEPTION_ERR;
	
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
				if (Flash_write(flashmem, &meta_buffer, PSN, block_read_buffer[offset_index]) == COMPLETE)
					goto OVERWRITE_ERR;
			}

			if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
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

			if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
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
		if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
	}

	//��� ���� ���̺�� Spare Block ���̺� �󿡼� SWAP
	SWAP(flashmem->block_level_mapping_table[LBN], flashmem->spare_block_table->table_array[spare_block_table_index], tmp);

	goto END_SUCCESS;


HYBRID_LOG_DYNAMIC: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	PBN1 = flashmem->log_block_level_mapping_table[LBN][0]; //������ ����� ���� ��� ��ȣ(PBN1)
	PBN2 = flashmem->log_block_level_mapping_table[LBN][1]; //������ ����� ���� ��� ��ȣ(PBN2)
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31
	PBN1_write_proc = PBN2_write_proc = false;

	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
		goto HYBRID_LOG_DYNAMIC_PBN1_PROC;
	else if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
		goto HYBRID_LOG_DYNAMIC_PBN1_PROC;
	else if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 != DYNAMIC_MAPPING_INIT_VALUE)
		goto HYBRID_LOG_DYNAMIC_PBN1_PROC;
	else
		goto HYBRID_LOG_DYNAMIC_PBN1_PROC;

HYBRID_LOG_DYNAMIC_PBN1_PROC: //PBN1�� ���� ó�� ��ƾ
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31

	/*** 
		PBN2������ �� ������ ��ġ�� ������ PBN1���� Overwrite�Ǿ� PBN2���� ���ǰ� �ִ� ��ġ�� ��� 
		PBN1�� PBN2�� ���� �����Ϳ� ���Ͽ� 1ȸ�� Overwrite�� ������ Log Blockó�� ���
		Overwrite�� ���� ��ġ�� 2�� �߻� �� PBN1�� PBN2�� ���Ͽ� Merge���� �� PBN1�� ���Ҵ�
	***/
	if (PBN2 != DYNAMIC_MAPPING_INIT_VALUE) //PBN2�� �Ҵ�Ǿ� ������
	{
		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��

		//PBN2���� �ش� ������ ��ġ�� �Ҵ�Ǿ� ���� ���
		if (flashmem->offset_level_mapping_table[offset_level_table_index] != OFFSET_MAPPING_INIT_VALUE)
		{
			/***
				PBN1�� PBN2�� ���� �����Ϳ� ���Ͽ� 1ȸ�� Overwrite�� ������ Log Blockó�� ���
				Overwrite�� ���� ��ġ�� 2�� �߻� ��(��, PBN2���� �ش� ��ġ�� �̹� ��ȿȭ�Ǿ��� ���)
				PBN1�� PBN2�� ���Ͽ� Merge���� �� PBN1�� ���Ҵ�
			***/

			/***
				���� ������ ��ȿȭ, ���� �ش� ����� ��� �������� ��ȿȭ�Ǿ����� �ش� ��� ��ȿȭ
				�ش� ����� ��� �������� ��ȿȭ�Ǵ� ������ valid_sector�� false�� set�� �� Spare Area�� ����� �߻��ϰ�, �̿� ���� ��ȿ ������ count ����
				�ش� ����� ��� �������� Spare Area�� ���� ��� �������� ��ȿȭ�Ǿ�����, valid_block�� false�� set�� �� ����� ù ���� Spare Area�� ����� �߻��ϹǷ�, 
				��ȿ ������ ������ �ٽ� count�Ǿ� Overflow �߻� (Spare Area�� ó�� �Լ����� meta������ ���� ��ȿ ������ count����)
				---
				=> ��� ������ meta������ ��� �о���� �� �Ǻ� �� �Ѳ����� meta ���� ���� �� ���
			***/

			PBN2_block_meta_buffer_array = SPARE_reads(flashmem, PBN2);

			switch (PBN2_block_meta_buffer_array[flashmem->offset_level_mapping_table[offset_level_table_index]]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector])
			{
			case true: //PBN2�� ���� ��ġ ��ȿȭ
				PBN2_block_meta_buffer_array[flashmem->offset_level_mapping_table[offset_level_table_index]]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;
				flashmem->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE; //������ �ʱ�ȭ

				/*** �̿� ���� ����, PBN2�� ��� �����Ͱ� ��ȿȭ�Ǿ�����, PBN2 ��ȿȭ ***/
				is_invalid_block = true;
				for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
				{
					if(flashmem->offset_level_mapping_table[(PBN2 * BLOCK_PER_SECTOR) + offset_index] != OFFSET_MAPPING_INIT_VALUE)
					{
						is_invalid_block = false;
						break;
					}
				}

				if (is_invalid_block == true)
				{
					PBN2_block_meta_buffer_array[0]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] = false;

					//PBN2�� ��� ���� ���� ���̺� �󿡼� Unlink(���� ����)
					flashmem->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE;
				}

				SPARE_writes(flashmem, PBN2, PBN2_block_meta_buffer_array);

				/*** Deallocate block_meta_buffer_array ***/
				if (deallocate_block_meta_buffer_array(PBN2_block_meta_buffer_array) != SUCCESS)
					goto MEM_LEAK_ERR;

				if (update_victim_block_info(flashmem, false, PBN2, mapping_method) != SUCCESS)
					goto VICTIM_BLOCK_INFO_EXCEPTION_ERR;

				break;

			case false: //Overwrite�� ���� ��ġ�� 2�� �߻� �� (��, PBN2���� �ش� ��ġ�� �̹� ��ȿȭ�Ǿ��� ���)
				/*** Deallocate block_meta_buffer_array ***/
				if (deallocate_block_meta_buffer_array(PBN2_block_meta_buffer_array) != SUCCESS)
					goto MEM_LEAK_ERR;

				if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE || PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //Merge�� ���ؼ� PBN1, PBN2 ��� �����Ǿ� �־�� �Ѵ�.
					goto MERGE_COND_EXCEPTION_ERR;

				if (full_merge(flashmem, LBN, mapping_method) != SUCCESS) //Merge ���� �� PBN1�� �� �Ҵ�
					goto WRONG_META_ERR;

				goto HYBRID_LOG_DYNAMIC; //�� ����

				break;
			}
		}
	}
	
	/*** ����, PBN1�� �Ҵ�Ǿ� ���� ���� ���, Spare Block�� �ƴ� �� ����� �Ҵ� ***/
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE)
	{
		if (search_empty_normal_block(flashmem, PBN1, &PBN1_meta_buffer, mapping_method, table_type) != SUCCESS) //�� �Ϲ� ���(PBN)�� ���������� Ž���Ͽ� PBN ��, �ش� PBN�� meta���� �޾ƿ´�
		{
			//��� ���� ���� ��� Spare Block�� ���
			//���� ���Ⱑ �߻��� ����� ��ȿ ������, �� ��ġ�� �����ϰ� Spare block�� ���, ���ο� ������ ��� LBN�� �Ҵ�
			//��������
			goto END_NO_SPACE; //��� ���� ���� ����
		}
		flashmem->log_block_level_mapping_table[LBN][0] = PBN1;

		PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

		if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ��� �� ��� ���� ����
		{
			PBN1_meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;

			//PBN1�� �ش� ������ ��ġ�� ���
			PBN1_write_proc = true;
			goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC;
		}
		else //��� �� ��ġ�� ����� ù ��° ���Ͱ� �ƴϸ� �� ��� ������ ���� ����
		{
			PBN1_meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
			SPARE_write(flashmem, (PBN1 * BLOCK_PER_SECTOR), &PBN1_meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 

			if (deallocate_single_meta_buffer(&PBN1_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			//PBN1�� �ش� ������ ��ġ�� ���
			PBN1_write_proc = true;
			goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC;
		}
	}

	/***
		�ش� ����� ��ȿȭ���� ���� ���
		---
		1) �ش� ����� ����ִ� ��� => �߻����� ����
		2) �ش� ����� ������� ���� ���
			2-1) �ش� ������ ��ġ�� �� ���
			=> �ش� ��ġ ���
			2-2) �ش� ������ ��ġ�� ��ȿ�ϰ�, ������� ���� ��� (Overwrite)
			=> PBN1�� ������ ��ȿȭ, PBN2�� ���
			2-2) �ش� ������ ��ġ�� ��ȿ�ϰ�, ������� ���� ��� (Overwrite)
			=> PBN2�� ������ ��ȿȭ, PBN2�� ���
	***/

	/*** �ش� ����� ������� ���� ��� ***/
	PBN1_block_meta_buffer_array = SPARE_reads(flashmem, PBN1);
	PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

	/*** �ش� ������ ��ġ�� �� ��� ***/
	if (PBN1_block_meta_buffer_array[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] == true)
	{

#if DEBUG_MODE == 1
		//����ִµ� ��ȿ���� ������
		if (PBN1_block_meta_buffer_array[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true)
			goto WRONG_META_ERR; //�߸��� meta ���� ����
#endif

		//PBN1�� �ش� ������ ��ġ�� ���
		PBN1_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC;
	}
	/*** �ش� ������ ��ġ�� ��ȿ�ϰ�, ������� ���� ��� ***/
	else if (PBN1_block_meta_buffer_array[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
		PBN1_block_meta_buffer_array[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
	{

		/*** PBN1�� ���� ������ ��ȿȭ ***/
		PBN1_block_meta_buffer_array[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;

		/*** �̿� ����, ����, PBN1�� ��� �����Ͱ� ��ȿȭ�Ǿ�����, PBN1 ��ȿȭ ***/
		is_invalid_block = true;
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			if (PBN1_block_meta_buffer_array[offset_index]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] == true)
			{
				is_invalid_block = false;
				break;
			}
		}

		if (is_invalid_block == true)
		{
			PBN1_block_meta_buffer_array[0]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] = false;

			//PBN1�� ��� ���� ���� ���̺� �󿡼� Unlink(���� ����)
			flashmem->log_block_level_mapping_table[LBN][0] = DYNAMIC_MAPPING_INIT_VALUE;
		}

		SPARE_writes(flashmem, PBN1, PBN1_block_meta_buffer_array);

		/*** Deallocate block_meta_buffer_array ***/
		if (deallocate_block_meta_buffer_array(PBN1_block_meta_buffer_array) != SUCCESS)
			goto MEM_LEAK_ERR;

		if (update_victim_block_info(flashmem, false, PBN1, mapping_method) != SUCCESS)
			goto VICTIM_BLOCK_INFO_EXCEPTION_ERR;
		
		///////////////////////////////////////////gc�����ٸ��ñ�,����ó���ʿ�?
		//flashmem->gc->scheduler(flashmem, mapping_method);

		/*** PBN2�� ���ο� ������ ��� ���� ***/
		goto HYBRID_LOG_DYNAMIC_PBN2_PROC;
	}
	/*** �ش� ������ ��ġ�� ��ȿ�ϰ�, ������� ���� ��� ***/
	else if (PBN1_block_meta_buffer_array[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::empty_sector] != true &&
		PBN1_block_meta_buffer_array[Poffset]->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] != true)
	{
		/*** Deallocate block_meta_buffer_array ***/
		if (deallocate_block_meta_buffer_array(PBN1_block_meta_buffer_array) != SUCCESS)
			goto MEM_LEAK_ERR;
		
		//PBN2�� ������ ��ȿȭ, PBN2�� ���ο� ������ ��� ����
		goto HYBRID_LOG_DYNAMIC_PBN2_PROC;
	}
	else
		goto WRONG_META_ERR;

HYBRID_LOG_DYNAMIC_PBN2_PROC: //PBN2�� ���� ó�� ��ƾ
	/*** ����, PBN2�� �Ҵ�Ǿ� ���� ���� ���, Spare Block�� �ƴ� �� ����� �Ҵ� ***/
	if (PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
	{
		if (search_empty_normal_block(flashmem, PBN2, &PBN2_meta_buffer, mapping_method, table_type) != SUCCESS) //�� �Ϲ� ���(PBN)�� ���������� Ž���Ͽ� PBN ��, �ش� PBN�� meta���� �޾ƿ´�
		{
			//��� ���� ���� ��� Spare Block�� ���
			//���� ���Ⱑ �߻��� ����� ��ȿ ������, �� ��ġ�� �����ϰ� Spare block�� ���, ���ο� ������ ��� LBN�� �Ҵ�
			//��������
			goto END_NO_SPACE; //��� ���� ���� ����
		}
		flashmem->log_block_level_mapping_table[LBN][1] = PBN2;

		/*** ���������� ����ִ� ������ ��ġ�� ��� ***/
		if (search_empty_offset_in_block(flashmem, PBN2, Poffset, &PBN2_meta_buffer) == FAIL)
			goto WRONG_META_ERR;

		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
		flashmem->offset_level_mapping_table[offset_level_table_index] = Poffset; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
		PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

		if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ��� �� ��� ���� ����
		{
			PBN2_meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;

			//PBN2�� �ش� ������ ��ġ�� ���
			PBN2_write_proc = true;
			goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC;
		}
		else //��� �� ��ġ�� ����� ù ��° ���Ͱ� �ƴϸ� �� ��� ������ ���� ����
		{
			PBN2_meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::empty_block] = false;
			SPARE_write(flashmem, (PBN2 * BLOCK_PER_SECTOR), &PBN2_meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 

			if (deallocate_single_meta_buffer(&PBN2_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			//PBN2�� �ش� ������ ��ġ�� ���
			PBN2_write_proc = true;
			goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC;
		}
	}

	/***
		�ش� ����� ��ȿȭ���� ���� ���
		---
		1) �ش� ����� ����ִ� ��� => �߻����� ����
		2) �ش� ����� ������� ���� ���
			2-1) ��� ���� �� �� ���� ���� ��
			=> ���������� �� ������ ���
			2-2) ��� ���� �� �� ���� �������� ���� ��
			=> PBN1�� PBN2�� ������ �� Spare Block�� ����Ͽ� ��ȿ ������ Merge ����, PBN1�� �� �Ҵ�
	***/

	/*** PBN2������ �� ������ ��ġ�� PBN1���� Overwrite�Ǿ� ���ǰ� �ִ� ��ġ�� ��� ***/
	offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
	Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
	if (Poffset != DYNAMIC_MAPPING_INIT_VALUE)
	{
		//PBN2�� ���� ��ġ ��ȿȭ
		PBN2_meta_buffer = SPARE_read(flashmem, PSN);
		PBN2_meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_sector] = false;
		SPARE_write(flashmem, PSN, &PBN2_meta_buffer);
		flashmem->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE; //������ �ʱ�ȭ

		if (deallocate_single_meta_buffer(&PBN2_meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		/*** �̿� ����, ����, PBN2�� ��� �����Ͱ� ��ȿȭ�Ǿ�����, PBN2 ��ȿȭ ***/
		if (update_victim_block_info(flashmem, false, PBN2, mapping_method) != SUCCESS)
			goto VICTIM_BLOCK_INFO_EXCEPTION_ERR;


		/* ��������
		if (flashmem->victim_block_info.victim_block_invalid_ratio == 1.0)
		{
			PBN2_meta_buffer = SPARE_read(flashmem, (PBN2 * BLOCK_PER_SECTOR));
			PBN2_meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::valid_block] = false;
			SPARE_write(flashmem, (PBN2 * BLOCK_PER_SECTOR), &PBN2_meta_buffer);

			if (deallocate_single_meta_buffer(&PBN2_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			//PBN2�� ��� ���� ���� ���̺� �󿡼� Unlink(���� ����)
			flashmem->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE;

			//flashmem->gc->scheduler(flashmem, mapping_method);
		}
		else //GC�� ���� Victim Block���� �������� �ʵ��� ����ü �ʱ�ȭ
			flashmem->victim_block_info.clear_all();
		*/
	}

	if (search_empty_offset_in_block(flashmem, PBN2, Poffset, &PBN2_meta_buffer) == FAIL)
	{
		//Merge�� ���ؼ� PBN1, PBN2 ��� �����Ǿ� �־�� �Ѵ�.
		if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE || PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
			goto MERGE_COND_EXCEPTION_ERR;

		if (full_merge(flashmem, LBN, mapping_method) != SUCCESS)
			goto WRONG_META_ERR;
		
		goto HYBRID_LOG_DYNAMIC; //�� ����
	}

	flashmem->offset_level_mapping_table[offset_level_table_index] = Poffset; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
	PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

	//�ش� ������ ��ġ�� ���
	PBN2_write_proc = true;
	goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC;

HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC: //���̺긮�� ���� ���� ��� ó�� ��ƾ
	if (PBN1_write_proc == true && PBN2_write_proc == false) //PBN1�� ���� ���
	{
		PBN1_write_proc = false;

		if ((PBN1_meta_buffer == NULL && PBN1_block_meta_buffer_array == NULL) || (PBN1_meta_buffer != NULL && PBN1_block_meta_buffer_array == NULL))
		{
			if(PBN1_meta_buffer == NULL)
				PBN1_meta_buffer = SPARE_read(flashmem, PSN);

			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, &PBN1_meta_buffer, PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;

			if (deallocate_single_meta_buffer(&PBN1_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

		}
		else if (PBN1_meta_buffer == NULL && PBN1_block_meta_buffer_array != NULL)
		{
			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, &PBN1_block_meta_buffer_array[Poffset], PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;

			if (deallocate_block_meta_buffer_array(PBN1_block_meta_buffer_array) != SUCCESS)
				goto MEM_LEAK_ERR;
		}
		else //PBN1_meta_buffer != NULL && PBN1_block_meta_buffer_array != NULL
			goto MEM_LEAK_ERR;
	}
	else if (PBN1_write_proc == false && PBN2_write_proc == true) //PBN2�� ���� ���
	{
		PBN2_write_proc = false;
		
		if ((PBN2_meta_buffer == NULL && PBN2_block_meta_buffer_array == NULL) || (PBN2_meta_buffer != NULL && PBN2_block_meta_buffer_array == NULL))
		{
			if (PBN2_meta_buffer == NULL)
				PBN2_meta_buffer = SPARE_read(flashmem, PSN);

			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, &PBN2_meta_buffer, PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;

			if (deallocate_single_meta_buffer(&PBN2_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

		}
		else if (PBN2_meta_buffer == NULL && PBN2_block_meta_buffer_array != NULL)
		{
			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, &PBN2_block_meta_buffer_array[Poffset], PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;

			if (deallocate_block_meta_buffer_array(PBN2_block_meta_buffer_array) != SUCCESS)
				goto MEM_LEAK_ERR;
		}
		else //PBN2_meta_buffer != NULL && PBN2_block_meta_buffer_array != NULL
			goto MEM_LEAK_ERR;
	}
	else
		goto WRITE_COND_EXCEPTION_ERR;

	goto END_SUCCESS; //����


END_SUCCESS: //���� ����
	if (meta_buffer != NULL || PBN1_meta_buffer != NULL || PBN2_meta_buffer != NULL || PBN1_block_meta_buffer_array != NULL || PBN2_block_meta_buffer_array != NULL)
		goto MEM_LEAK_ERR;

	flashmem->save_table(mapping_method);
	
	return SUCCESS;

END_NO_SPACE: //��� ���� ���� ����
	fprintf(stderr, "��� �� ������ �������� ���� : �Ҵ�� ũ���� ���� ��� ���\n");
	return FAIL;

	/*����
VICTIM_BLOCK_INFO_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : Victim Block ���� ���� ���� - �̹� ��� �� (FTL_write)\n");
	system("pause");
	exit(1);
	*/
SPARE_BLOCK_EXCEPTION_ERR: //Spare Block Table�� ���� ����
	if(VICTIM_BLOCK_QUEUE_RATIO != SPARE_BLOCK_RATIO)
		fprintf(stderr, "Spare Block Table�� �Ҵ�� ũ���� ���� ��� ��� : �̱���, GC�� ���� ó���ǵ��� �ؾ��Ѵ�.\n");
	else
	{
		fprintf(stderr, "ġ���� ���� : Spare Block Table �� GC Scheduler�� ���� ���� �߻� (FTL_write)\n");
		system("pause");
		exit(1);
	}
	return FAIL;


/*����
WRONG_META_ERR: //�߸��� meta���� ����
	fprintf(stderr, "ġ���� ���� : �߸��� meta ���� (FTL_write)\n");
	system("pause");
	exit(1);
	*/

WRONG_MAPPING_METHOD_ERR:
	fprintf(stderr, "ġ���� ���� : �߸��� ���� ��� (FTL_write)\n");
	system("pause");
	exit(1);

WRONG_TABLE_TYPE_ERR: //�߸��� ���̺� Ÿ��
	fprintf(stderr, "ġ���� ���� : �߸��� ���̺� Ÿ�� (FTL_write)\n");
	system("pause");
	exit(1);

WRONG_STATIC_TABLE_ERR: //�߸��� ���� ���̺� ����
	fprintf(stderr, "ġ���� ���� : ���� ���̺� ���� �������� ���� ���̺�\n");
	system("pause");
	exit(1);

WRITE_COND_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ���� ����\n");
	system("pause");
	exit(1);
	
MERGE_COND_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : Merge ���� ���� (PBN 1 : %u , PBN2 : %u)\n", PBN1, PBN2);
	system("pause");
	exit(1);

OVERWRITE_ERR: //Overwrite ����
	fprintf(stderr, "ġ���� ���� : Overwrite�� ���� ���� �߻� (FTL_write)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (FTL_write)\n");
	system("pause");
	exit(1);
}

int full_merge(FlashMem*& flashmem, unsigned int LBN, MAPPING_METHOD mapping_method) //Ư�� LBN�� ������ PBN1�� PBN2�� ���Ͽ� Merge ����
{
	unsigned int PBN1 = flashmem->log_block_level_mapping_table[LBN][0]; //LBN�� ������ ���� ���(PBN1)
	unsigned int PBN2 = flashmem->log_block_level_mapping_table[LBN][1]; //LBN�� ������ ���� ���(PBN2)
	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ

	__int8 Loffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ���� ���� Offset = 0 ~ 31
	__int8 Poffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ������ ���� Offset = 0 ~ 31
	unsigned int offset_level_table_index = DYNAMIC_MAPPING_INIT_VALUE; //������ ���� ���̺� �ε���

	unsigned int empty_spare_block = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare ���
	unsigned int spare_block_table_index = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare �����  Spare ��� ���̺� �� �ε���
	unsigned int tmp = DYNAMIC_MAPPING_INIT_VALUE; //���̺� SWAP���� �ӽ� ����

	char read_buffer = NULL; //���� ���ͷκ��� �о���� ������
	char block_read_buffer[BLOCK_PER_SECTOR] = { NULL }; //�� ��� ���� ������ �ӽ� ���� ����
	__int8 read_buffer_index = 0; //�����͸� �о �ӽ������ϱ� ���� read_buffer �ε��� ����

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	switch (mapping_method) //���� ����
	{
	case MAPPING_METHOD::HYBRID_LOG:
		break;

	default:
		return FAIL;
	}

	f_flash_info = flashmem->get_f_flash_info();

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
	//if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE || PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
		return COMPLETE; //�ش� �� ��Ͽ� ���Ͽ� Merge �Ұ�

	printf("Performing Merge on PBN1 : %u and PBN2 : %u...\n", PBN1, PBN2);
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
		if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		PSN = (PBN2 * BLOCK_PER_SECTOR);
		meta_buffer = SPARE_read(flashmem, PSN);
		//PBN2�� Spare ����� �ƴ� ��쿡
		if (meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] == true)
		{
			if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
		}
		else //Spare ����� ���
			goto WRONG_META_ERR;

	}
	else //Spare ����� ���
		goto WRONG_META_ERR;
#endif

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
			Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //���� ��� ���� ������ ������ ��ġ
			PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset; //������ ����� ���� ���� ��ȣ

			Flash_read(flashmem, NULL, PSN, block_read_buffer[Loffset]);
		}

		if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
	}

	/*** PBN1, PBN2 Erase�� �ϳ���(PBN1) Spare ������� ���� ***/
	Flash_erase(flashmem, PBN1); //PBN1�� erase
	Flash_erase(flashmem, PBN2); //PBN2�� erase
	PSN = PBN1 * BLOCK_PER_SECTOR; //�ش� ����� ù ��° ����(������)
	meta_buffer = SPARE_read(flashmem, PSN);
	meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = false;
	SPARE_write(flashmem, PSN, &meta_buffer);
	if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;

	/*** �� Spare ����� ã�Ƽ� ��� ***/
	if (flashmem->spare_block_table->rr_read(flashmem, empty_spare_block, spare_block_table_index) == FAIL)
		goto SPARE_BLOCK_EXCEPTION_ERR;

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

			if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
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
		if (deallocate_single_meta_buffer(&meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
	}

	/*** PBN2 ��� ���� ���̺�, ������ ���� ���̺� �ʱ�ȭ ***/
	for (Loffset = 0; Loffset < BLOCK_PER_SECTOR; Loffset++)
	{
		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� index��
		flashmem->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE;
	}
	PBN2 = flashmem->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE;

	/*** PBN1�� Spare ��� ���̺� ���� ***/
	//PBN1�� �������� �׻� ��ġ��Ű���� �Ͽ����Ƿ�, ��� ���� ���̺� ����
	SWAP(flashmem->log_block_level_mapping_table[LBN][0], flashmem->spare_block_table->table_array[spare_block_table_index], tmp); //PBN1�� Spare ��� ��ü

	flashmem->save_table(mapping_method);
	return SUCCESS;

SPARE_BLOCK_EXCEPTION_ERR:
	if (VICTIM_BLOCK_QUEUE_RATIO != SPARE_BLOCK_RATIO)
		fprintf(stderr, "Spare Block Table�� �Ҵ�� ũ���� ���� ��� ��� : �̱���, GC�� ���� ó���ǵ��� �ؾ��Ѵ�.\n");
	else
	{
		fprintf(stderr, "ġ���� ���� : Spare Block Table �� GC Scheduler�� ���� ���� �߻� (FTL_write)\n");
		system("pause");
		exit(1);
	}
	return FAIL;

WRONG_META_ERR: //�߸��� meta���� ����
	fprintf(stderr, "ġ���� ���� : �߸��� meta ���� (full_merge)\n");
	system("pause");
	exit(1);

OVERWRITE_ERR: //Overwrite ����
	fprintf(stderr, "ġ���� ���� : Overwrite�� ���� ���� �߻� (full_merge)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (full_merge)\n");
	system("pause");
	exit(1);
}

int full_merge(FlashMem*& flashmem, MAPPING_METHOD mapping_method) //���̺��� �����Ǵ� ��� ��Ͽ� ���� Merge ����
{
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	switch (mapping_method) //���� ����
	{
	case MAPPING_METHOD::HYBRID_LOG:
		break;

	default:
		return FAIL;
	}

	f_flash_info = flashmem->get_f_flash_info();

	for (unsigned int LBN = 0; LBN < (f_flash_info.block_size - f_flash_info.spare_block_size); LBN++)
	{
		//�Ϲ� ��� ���� ���� ���̺��� �����Ǵ� ��� �Ϲ� ��Ͽ� ���� ������ ��� Merge ����
		if (full_merge(flashmem, LBN, mapping_method) == FAIL)
			return FAIL;
	}

	return SUCCESS;
}

int trace(FlashMem*& flashmem, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //Ư�� ���Ͽ� ���� ���� ������ �����ϴ� �Լ�
{
	//��ü trace �� �����ϱ� ���� ����� read, write, erase Ƚ�� ���

	FILE* trace_file_input = NULL; //trace ���� �Է� ����
	
#if BLOCK_TRACE_MODE == 1 //Trace for Per Block Wear-leveling
	FILE* trace_per_block_output = NULL; //��� �� trace ��� ���
	F_FLASH_INFO f_flash_info;
	f_flash_info = flashmem->get_f_flash_info();
#endif

	char file_name[2048]; //trace ���� �̸�
	char op_code[2] = { 0, }; //�����ڵ�(w,r,e) : '\0' ���� 2�ڸ�
	unsigned int LSN = UINT32_MAX; //LSN
	char dummy_data = 'A'; //trace�� ���� ���� ������
	
	system("cls");
	std::cout << "< ���� ���� ��� >" << std::endl;
	system("dir");
	std::cout << "trace ���� �̸� �Է� (�̸�.Ȯ����) >>";
	gets_s(file_name, 2048);

	trace_file_input = fopen(file_name, "rt"); //�б� + �ؽ�Ʈ ���� ���

	if (trace_file_input == NULL)
	{
		fprintf(stderr, "File does not exist or cannot be opened.\n");
		return FAIL;
	}

	std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now(); //trace �ð� ���� ����
	while (!feof(trace_file_input))
	{
		memset(op_code, NULL, sizeof(op_code)); //�����ڵ� ���� �ʱ�ȭ
		LSN = UINT32_MAX; //���� ������ �÷��� �޸��� �뷮���� ���� �� ���� ���� LSN ������ �ʱ�ȭ

		fscanf(trace_file_input, "%s\t%u\n", &op_code, &LSN); //������ �и��� ���� �б�
		if (strcmp(op_code, "w") == 0 || strcmp(op_code, "W") == 0)
		{
			FTL_write(flashmem, LSN, dummy_data, mapping_method, table_type);
		}
	}
	fclose(trace_file_input);
	std::chrono::system_clock::time_point end_time = std::chrono::system_clock::now(); //trace �ð� ���� ��
	std::chrono::milliseconds mill_end_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << ">> Trace function ends : " << mill_end_time.count() <<"milliseconds elapsed"<< std::endl;

#if BLOCK_TRACE_MODE == 1 //Trace for Per Block Wear-leveling
	trace_per_block_output = fopen("trace_per_block_result.txt", "wt");
	if (trace_per_block_output == NULL)
	{
		fprintf(stderr, "��� �� ���� ��� ���� ���� (trace_per_block_result.txt)\n");
		return FAIL;
	}

	fprintf(trace_per_block_output, "PBN\tRead\tWrite\tErase\n");
	for (unsigned int PBN = 0; PBN < f_flash_info.block_size; PBN++)
	{
		//PBN [TAB] �б� Ƚ�� [TAB] ���� Ƚ�� [TAB] ����� Ƚ�� ��� �� ���� trace�� ���� �ʱ�ȭ ���� ����
		fprintf(trace_per_block_output, "%u\t%u\t%u\t%u\n", PBN, flashmem->block_trace_info[PBN].block_read_count, flashmem->block_trace_info[PBN].block_write_count, flashmem->block_trace_info[PBN].block_erase_count);
		flashmem->block_trace_info[PBN].clear_all(); //�б�, ����, ����� Ƚ�� �ʱ�ȭ
	}
	fclose(trace_per_block_output);

	printf(">> ��� �� ���� ���� trace_per_block_result.txt\n");
	system("notepad trace_per_block_result.txt");

#endif

	return SUCCESS;
}