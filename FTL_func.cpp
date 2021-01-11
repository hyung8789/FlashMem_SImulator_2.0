#include "Build_Options.h"

// Print_table, FTL_read, FTL_write, trace ����
// �� ���� ��ȣ �Ǵ� �� ��� ��ȣ�� ���� ���̺�� �����Ͽ� physical_func���� read, write, erase�� �����Ͽ� �۾��� ����

int Print_table(FlashMem*& flashmem, MAPPING_METHOD mapping_method, TABLE_TYPE table_type)  //���� ���̺� ���
{
	FILE* table_output = NULL; //���̺��� ���Ϸ� ����ϱ� ���� ���� ������

	unsigned int block_table_size = 0; //��� ���� ���̺��� ũ��
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
		/*
		if (table_type == TABLE_TYPE::DYNAMIC) //Static Table�� ���, ���� �߻� �� �� ��� �Ҵ� ������ �ʿ� �����Ƿ� Empty Block Queue�� ������� �ʴ´�.
		{
			std::cout << "\n< Empty Block Queue (Index -> PBN) >" << std::endl;
			fprintf(table_output, "\n< Empty Block Queue (Index -> PBN) >\n");
			table_index = 0;
			while (table_index < f_flash_info.spare_block_size)
			{
				printf("%u -> %u\n", table_index, flashmem->empty_block_queue->queue_array[table_index]);
				fprintf(table_output, "%u -> %u\n", table_index, flashmem->empty_block_queue->queue_array[table_index]);

				table_index++;
			}
		}
		*/
		break;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		block_table_size = f_flash_info.block_size - f_flash_info.spare_block_size; //�Ϲ� ��� ��
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

int FTL_read(FlashMem*& flashmem, unsigned int LSN, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //���� ���̺��� ���� LSN �б�
{
	unsigned int LBN = DYNAMIC_MAPPING_INIT_VALUE; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	unsigned int PBN = DYNAMIC_MAPPING_INIT_VALUE; //�ش� ���� ���Ͱ� ��ġ�ϰ� �ִ� ���� ���
	unsigned int PBN1 = DYNAMIC_MAPPING_INIT_VALUE; //������ ���
	unsigned int PBN2 = DYNAMIC_MAPPING_INIT_VALUE; //�α� ���
	unsigned int PSN = DYNAMIC_MAPPING_INIT_VALUE; //������ ����� ���� ���� ��ȣ
	unsigned int offset_level_table_index = DYNAMIC_MAPPING_INIT_VALUE; //������ ���� ���̺� �ε���

	__int8 Loffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ���� ���� Offset = 0 ~ 31
	__int8 Poffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ������ ���� Offset = 0 ~ 31

	bool hybrid_log_dynamic_both_assigned = false; //LBN�� ���� PBN1, PBN2 �� �� ��� ���� ����

	char read_buffer = NULL; //���� ���ͷκ��� �о���� ������

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	if (flashmem == NULL) //�÷��� �޸𸮰� �Ҵ���� �ʾ��� ���
	{
		fprintf(stderr, "not initialized\n");
		return FAIL;
	}
	f_flash_info = flashmem->get_f_flash_info(); //������ �÷��� �޸��� ������ ������ �����´�

	switch (flashmem->v_flash_info.flash_state)
	{
	case FLASH_STATE::IDLE:
		flashmem->v_flash_info.flash_state = FLASH_STATE::WRITE; //���� �۾� ������ �˸�

	default:
		break;
	}

	//�ý��ۿ��� ����ϴ� Spare Block�� ����(������)����ŭ ����
	if (LSN > (unsigned int)((MB_PER_SECTOR * f_flash_info.flashmem_size) - (f_flash_info.spare_block_size * BLOCK_PER_SECTOR) - 1)) //���� �ʰ� ����
	{
		fprintf(stderr, "out of range error\n");
		return FAIL;
	}
	
	switch (mapping_method)
	{
	case MAPPING_METHOD::BLOCK: //��� ����
		goto BLOCK_MAPPING_COMMON_READ_PROC;

	case MAPPING_METHOD::HYBRID_LOG: //���̺긮�� ���� (log algorithm - 1:2 block level mapping)
		goto HYBRID_LOG_DYNAMIC;

	default:
		return FAIL;
	}

BLOCK_MAPPING_COMMON_READ_PROC: //��� ���ο� ���� ���� �б� ó�� ��ƾ
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	PBN = flashmem->block_level_mapping_table[LBN];
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
	PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //������ ����� ���� ���� ��ȣ

	if (PBN == DYNAMIC_MAPPING_INIT_VALUE) //Dynamic Table �ʱⰪ�� ���
		goto NON_ASSIGNED_LBN;

#ifdef DEBUG_MODE //��ȿȭ�� ��� �Ǻ� - GC�� ���� ���� ó������ �ʾ��� ��� ���� �׽�Ʈ : �Ϲ� ��� ���� ���̺� �����Ǿ� �־�� �ȵ�
	if (PSN % BLOCK_PER_SECTOR == 0) //���� ��ġ�� ����� ù ��° ������ ���
	{
		SPARE_read(flashmem, PSN, meta_buffer);

		switch (meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			break;

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
		SPARE_read(flashmem, (PBN * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����

		switch (meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
			goto INVALID_BLOCK_ERR;

		default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}

	SPARE_read(flashmem, PSN, meta_buffer); //���� ��ġ

	switch (meta_buffer->get_sector_state()) //���� ��ġ�� ���°� ��ȿ �� ��츸 ����
	{
	case SECTOR_STATE::EMPTY:
		goto EMPTY_PAGE;

	case SECTOR_STATE::INVALID:
		goto INVALID_PAGE_ERR;

	case SECTOR_STATE::VALID:
		Flash_read(flashmem, meta_buffer, PSN, read_buffer); //�����͸� �о��

		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		goto OUTPUT_DATA_SUCCESS;
	}
#else
	SPARE_read(flashmem, PSN, meta_buffer); //���� ��ġ

	switch (meta_buffer->get_sector_state()) //���� ��ġ�� ���°� ��ȿ �� ��츸 ����
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

HYBRID_LOG_DYNAMIC:
	/***
		PBN1 : �������� �׻� ��ġ��Ŵ
		PBN2 : ������ ���� ���̺��� ���Ͽ� �о����
	***/
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	PBN1 = flashmem->log_block_level_mapping_table[LBN][0]; //PBN1
	PBN2 = flashmem->log_block_level_mapping_table[LBN][1]; //PBN2

	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //PBN1, PBN2 ��� �Ҵ� ���� ���� ���
		goto NON_ASSIGNED_LBN;
	else if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE) //PBN1�� �Ҵ�� ���
		goto HYBRID_LOG_DYNAMIC_PBN1_PROC;
	else if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 != DYNAMIC_MAPPING_INIT_VALUE) //PBN2�� �Ҵ�� ���
		goto HYBRID_LOG_DYNAMIC_PBN2_PROC;
	else //PBN1, PBN2 ��� �Ҵ�� ���
	{
		/***
			���� PBN2�� �д´�.
			������ ���� ���̺��� �׻� ��ȿ�� �����͸� ����Ų��.
			�̿� ����, ���� PBN2�� ������ ���� ���̺��� Poffset�� OFFSET_MAPPING_INIT_VALUE �� ���,
			PBN1�� PBN2�� ��� �Ҵ�Ǿ� �ִ� ��Ȳ���� �ش� LSN�� ��ȿ�� �����ʹ� PBN1�� �������� �ǹ��Ѵ�.
		***/
		hybrid_log_dynamic_both_assigned = true; //LBN�� ���Ͽ� PBN1, PBN2 ��� �����Ǿ����� �˸�
		goto HYBRID_LOG_DYNAMIC_PBN2_PROC;
	}

HYBRID_LOG_DYNAMIC_PBN1_PROC: //PBN1�� ó�� ��ƾ
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR;
	PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset;
	
#ifdef DEBUG_MODE //��ȿȭ�� ��� �Ǻ� - GC�� ���� ���� ó������ �ʾ��� ��� ���� �׽�Ʈ : �Ϲ� ��� ���� ���̺� �����Ǿ� �־�� �ȵ�
	if (PSN % BLOCK_PER_SECTOR == 0) //���� ��ġ�� ����� ù ��° ������ ���
	{
		SPARE_read(flashmem, PSN, meta_buffer);

		switch (meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			break;

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
		SPARE_read(flashmem, (PBN1 * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����

		switch (meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
			goto INVALID_BLOCK_ERR;

		default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}

	SPARE_read(flashmem, PSN, meta_buffer); //���� ��ġ

	switch (meta_buffer->get_sector_state()) //���� ��ġ�� ���°� ��ȿ �� ��츸 ����
	{
	case SECTOR_STATE::EMPTY:
		if (hybrid_log_dynamic_both_assigned) //PBN1, PBN2 ��� �Ҵ�Ǿ� �ִ� ��Ȳ���� PBN2�� ���� �Ǻ��� ������ PBN1 ó�� ��ƾ���� �Ѿ�� ���
			goto HYBRID_LOG_BOTH_ASSIGNED_EXCEPTION_ERR; //PBN2���� ����ִ� ��ġ��� PBN1���� �ݵ�� ��ȿ�� �����Ͱ� �����ؾ� �Ѵ�.

		goto EMPTY_PAGE;

	case SECTOR_STATE::INVALID:
		if (hybrid_log_dynamic_both_assigned) //PBN1, PBN2 ��� �Ҵ�Ǿ� �ִ� ��Ȳ���� PBN2�� ���� �Ǻ��� ������ PBN1 ó�� ��ƾ���� �Ѿ�� ���
			goto HYBRID_LOG_BOTH_ASSIGNED_EXCEPTION_ERR; //PBN2���� ����ִ� ��ġ��� PBN1���� �ݵ�� ��ȿ�� �����Ͱ� �����ؾ� �Ѵ�.

		goto INVALID_PAGE_ERR;

	case SECTOR_STATE::VALID:
		Flash_read(flashmem, meta_buffer, PSN, read_buffer); //�����͸� �о��

		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		goto OUTPUT_DATA_SUCCESS;
	}
#else
	SPARE_read(flashmem, PSN, meta_buffer); //���� ��ġ

	switch (meta_buffer->get_sector_state()) //���� ��ġ�� ���°� ��ȿ �� ��츸 ����
	{
	case SECTOR_STATE::EMPTY:
		if (hybrid_log_dynamic_both_assigned) //PBN1, PBN2 ��� �Ҵ�Ǿ� �ִ� ��Ȳ���� PBN2�� ���� �Ǻ��� ������ PBN1 ó�� ��ƾ���� �Ѿ�� ���
			goto HYBRID_LOG_BOTH_ASSIGNED_EXCEPTION_ERR; //PBN2���� ����ִ� ��ġ��� PBN1���� �ݵ�� ��ȿ�� �����Ͱ� �����ؾ� �Ѵ�.

		goto EMPTY_PAGE;
	
	case SECTOR_STATE::INVALID:
		if (hybrid_log_dynamic_both_assigned) //PBN1, PBN2 ��� �Ҵ�Ǿ� �ִ� ��Ȳ���� PBN2�� ���� �Ǻ��� ������ PBN1 ó�� ��ƾ���� �Ѿ�� ���
			goto HYBRID_LOG_BOTH_ASSIGNED_EXCEPTION_ERR; //PBN2���� ����ִ� ��ġ��� PBN1���� �ݵ�� ��ȿ�� �����Ͱ� �����ؾ� �Ѵ�.

		goto INVALID_PAGE_ERR;

	case SECTOR_STATE::VALID:
		Flash_read(flashmem, DO_NOT_READ_META_DATA, PSN, read_buffer); //�����͸� �о��

		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		goto OUTPUT_DATA_SUCCESS;
	}
#endif

HYBRID_LOG_DYNAMIC_PBN2_PROC: //PBN2�� ó�� ��ƾ
	Loffset = LSN % BLOCK_PER_SECTOR; //������ ���� ���̺� �������� LSN�� �� ������
	offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
	Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
	PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

#ifdef DEBUG_MODE //��ȿȭ�� ��� �Ǻ� - GC�� ���� ���� ó������ �ʾ��� ��� ���� �׽�Ʈ : �Ϲ� ��� ���� ���̺� �����Ǿ� �־�� �ȵ�
	if (Poffset != OFFSET_MAPPING_INIT_VALUE)
	{
		if (PSN % BLOCK_PER_SECTOR == 0) //���� ��ġ�� ����� ù ��° ������ ���
		{
			SPARE_read(flashmem, PSN, meta_buffer);

			switch (meta_buffer->get_block_state())
			{
			case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
				break;

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
			SPARE_read(flashmem, (PBN2 * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����

			switch (meta_buffer->get_block_state())
			{
			case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;
				break;

			case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
				if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
					goto MEM_LEAK_ERR;
				break;

			case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
				goto INVALID_BLOCK_ERR;

			default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
				goto WRONG_ASSIGNED_LBN_ERR;
			}
		}
	}
	else //Poffset�� �Ҵ���� ���� ���, ��� ������ ��� ���� ���� ����
	{
		SPARE_read(flashmem, (PBN2* BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����

		switch (meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
			goto INVALID_BLOCK_ERR;

		default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}
#endif
	
	/***
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
	{
		if (hybrid_log_dynamic_both_assigned) //PBN1, PBN2 ��� �Ҵ�Ǿ� ���� ���
			goto HYBRID_LOG_DYNAMIC_PBN1_PROC; //PBN1�� PBN2�� ��� �Ҵ�Ǿ� �ִ� ��Ȳ���� �ش� LSN�� ��ȿ�� �����ʹ� PBN1�� ����, PBN1 ó�� ��ƾ���� �̵�

		goto EMPTY_PAGE;
	}

OUTPUT_DATA_SUCCESS:
	if (meta_buffer != NULL)
		goto MEM_LEAK_ERR;

	std::cout << "PSN : " << PSN << std::endl; //���� ���� ��ȣ ���
	std::cout << read_buffer << std::endl;

	return SUCCESS;

EMPTY_PAGE:
	std::cout << "no data (Empty Page)" << std::endl;

	return COMPLETE;

NON_ASSIGNED_LBN:
	std::cout << "no data (Non-Assigned LBN)" << std::endl;

	return COMPLETE;

HYBRID_LOG_BOTH_ASSIGNED_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : PBN2���� �Ǻ� �Ϸ� �� PBN1�� �Ǻ� �Ͽ�����, PBN1�� ��ȿ ������ �������� ���� (FTL_read)\n");
	system("pause");
	exit(1);

WRONG_ASSIGNED_LBN_ERR:
	fprintf(stderr, "ġ���� ���� : �߸� �Ҵ� �� LBN (FTL_read)\n");
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

int FTL_write(FlashMem*& flashmem, unsigned int LSN, const char src_data, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //���� ���̺��� ���� LSN ����
{
	char block_read_buffer[BLOCK_PER_SECTOR] = { NULL, }; //�� ��� ���� ������ �ӽ� ���� ����
	__int8 read_buffer_index = 0; //�����͸� �о �ӽ������ϱ� ���� read_buffer �ε��� ����
	
	unsigned int empty_normal_block_num = DYNAMIC_MAPPING_INIT_VALUE; //����� �� �Ϲ� ���
	unsigned int empty_spare_block_num = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare ���
	unsigned int spare_block_queue_index = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare ����� Spare Block Queue �� �ε���
	unsigned int PBN_for_overwrite_proc = DYNAMIC_MAPPING_INIT_VALUE; //���̺긮�� ���� ���� Overwrite ó�� ��ƾ�� ���� PBN ���� ����
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
	META_DATA** PBN_block_meta_buffer_array = NULL; // �� ���� ��� ���� ��� ����(������)�� ���� meta ���� (PBN)
	META_DATA** PBN1_block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� meta ���� (PBN1)
	META_DATA** PBN2_block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� meta ���� (PBN2)

	bool PBN1_write_proc = false; //PBN1�� ���� ���� �۾� ���� ���� ����
	bool PBN2_write_proc = false; //PBN2�� ���� ���� �۾� ���� ���� ����
	bool flag_merge_performed = false; //��ü ��Ͽ� ���� Merge ���� �߻��Ͽ����� ����
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
	PSN = (PBN * BLOCK_PER_SECTOR) + Poffset;

#ifdef DEBUG_MODE
	//���� ���̺��� �׻� �� ��� Ȥ�� ��ȿ�� ������� �����Ǿ� �־�� ��
	if (PBN == DYNAMIC_MAPPING_INIT_VALUE)
		goto WRONG_STATIC_TABLE_ERR;
#endif

	/*** 
		���� ���̺��� ���, �׻� �� ��� Ȥ�� ��ȿ�� ������� ������Ų��.
		�ʱ� : �� ���
		��� �߻� �� : ��ȿ�� ���
		Overwrite�� ���� Erase �߻� �� : �ٸ� �� ���
	***/

	if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ���
	{
		SPARE_read(flashmem, PSN, meta_buffer);

		switch (meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID); //�ش� ��Ͽ� ��� ���� �����̹Ƿ�, VALID ���·� ����
			break;

		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
			break;

		case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
			goto INVALID_BLOCK_ERR;

		default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}
	else //����� ù ��° ������ Spare ������ ���� �ش� ��� ���� �Ǻ� �� ����
	{
		SPARE_read(flashmem, (PBN * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����

		switch (meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID); //�ش� ��Ͽ� ��� ���� �����̹Ƿ�, VALID ���·� ����

			SPARE_write(flashmem, (PBN * BLOCK_PER_SECTOR), meta_buffer); //���� ������ �ϴ� ��ġ�� �ƴϹǷ�, ��� ���� ��� ����

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� �Ϲ� ����� ���
			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		case BLOCK_STATE::NORMAL_BLOCK_INVALID: //��ȿ���� ���� �Ϲ� ��� - GC�� ���� ���� ó���� ���� �ʾ��� ���
			goto INVALID_BLOCK_ERR;

		default: //Spare Block�� �Ҵ�Ǿ� �ִ� ���
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}
	
	if(meta_buffer == NULL) //��� �� ��ġ�� meta ������ �ǵ� �� �� ���
		SPARE_read(flashmem, PSN, meta_buffer); //��� �� ��ġ

	switch (meta_buffer->get_sector_state()) //��� �� ��ġ�� ���°� ��� ���� ��쿡 ���, �̹� ��ȿ ������ ���� �� Overwrite ó�� ��ƾ���� �̵�
	{
	case SECTOR_STATE::EMPTY:
		goto BLOCK_MAPPING_COMMON_WRITE_PROC; //�ش� ��ġ�� ���

	case SECTOR_STATE::INVALID: //���� ��ġ�� ���Ͽ� Overwrite �߻� �� �ش� ����� Erase ����ǹǷ� ��ȿ �������� �������� �ʴ´�.
		goto INVALID_PAGE_ERR;

	case SECTOR_STATE::VALID:
		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		goto BLOCK_MAPPING_COMMON_OVERWRITE_PROC; //Overwrite ó�� ��ƾ���� �̵�
	}

BLOCK_MAPPING_DYNAMIC: //��� ���� Dynamic Table
	//����ڰ� �Է��� LSN���� LBN�� ���ϰ� �����Ǵ� PBN�� ���� ���� ��ȣ�� ����
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� ���� offset = 0 ~ 31
	PBN = flashmem->block_level_mapping_table[LBN]; //������ ����� ���� ��� ��ȣ
	PSN = (PBN * BLOCK_PER_SECTOR) + Poffset;

	/*** LBN�� PBN�� �������� ���� ��� : �� ��� �Ҵ� ***/
	if (PBN == DYNAMIC_MAPPING_INIT_VALUE)
	{
		if(flashmem->empty_block_queue->dequeue(flashmem->block_level_mapping_table[LBN]) != SUCCESS)
			goto END_NO_SPACE; //��� ���� ���� ����

		//���ο� PBN �Ҵ翡 ���� �� ����
		PBN = flashmem->block_level_mapping_table[LBN];
		PSN = (PBN * BLOCK_PER_SECTOR) + Poffset; //��� �� ��ġ

		if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ��� �� ��� ���� ����
		{
			SPARE_read(flashmem, PSN, meta_buffer);

			switch (meta_buffer->get_block_state())
			{
			case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
				meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID); //�ش� ��Ͽ� ��� ���� �����̹Ƿ�, VALID ���·� ����
				break;

			default: //�� ����� �ƴ� ���
				goto WRONG_ASSIGNED_LBN_ERR;
			}
		}
		else //��� �� ��ġ�� ����� ù ��° ���Ͱ� �ƴϸ� �� ��� ������ ���� ����
		{
			SPARE_read(flashmem, (PBN* BLOCK_PER_SECTOR), meta_buffer);
			meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID);
			SPARE_write(flashmem, (PBN * BLOCK_PER_SECTOR), meta_buffer); //���� ������ �ϴ� ��ġ�� �ƴϹǷ�, ��� ���� ��� ����

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
		}
	}

	if (meta_buffer == NULL) //��� �� ��ġ�� meta ������ �ǵ� �� �� ���
		SPARE_read(flashmem, PSN, meta_buffer); //��� �� ��ġ

	switch (meta_buffer->get_sector_state()) //��� �� ��ġ�� ���°� ��� ���� ��쿡 ���, �̹� ��ȿ ������ ���� �� Overwrite ó�� ��ƾ���� �̵�
	{
	case SECTOR_STATE::EMPTY:
		goto BLOCK_MAPPING_COMMON_WRITE_PROC; //�ش� ��ġ�� ���

	case SECTOR_STATE::INVALID: //���� ��ġ�� ���Ͽ� Overwrite �߻� �� �ش� ����� Erase ����ǹǷ� ��ȿ �������� �������� �ʴ´�.
		goto INVALID_PAGE_ERR;

	case SECTOR_STATE::VALID:
		if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		goto BLOCK_MAPPING_COMMON_OVERWRITE_PROC; //Overwrite ó�� ��ƾ���� �̵�
	}

BLOCK_MAPPING_COMMON_WRITE_PROC: //��� ���� ���� ó�� ��ƾ 1 : ���ǰ� �ִ� ����� ��� �ִ� �����¿� ���� ��� ����
	if (meta_buffer == NULL) //��� �� ��ġ�� meta ������ �ǵ� �� �� ���
		SPARE_read(flashmem, PSN, meta_buffer); //��� �� ��ġ

	if (Flash_write(flashmem, meta_buffer, PSN, src_data) == COMPLETE)
		goto OVERWRITE_ERR;

	if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;

	goto END_SUCCESS; //����

BLOCK_MAPPING_COMMON_OVERWRITE_PROC: //��� ���� ���� ó�� ��ƾ 2 : ���ǰ� �ִ� ����� ���� ��ġ�� ���� Overwrite ����
	/***
		1) �ش� PBN�� ��ȿ�� ������(���ο� �����Ͱ� ��ϵ� ��ġ ����) �� ���ο� �����͸� ������ �� Spare Block�� ����Ͽ�
		��ȿ ������(���ο� �����Ͱ� ��ϵ� ��ġ ����) copy �� ���ο� ������ ���
		2) ���� PBN ���� ��� ���� �� �ش� ��� ��ȿȭ
		3) ���� PBN�� ���� Spare Block ���̺� �� ��ü �� ���� PBN�� Victim Block���� ����
	***/

	/*** ��ȿ ������ ���� (Overwrite�� ��ġ �� �� ��ġ�� ����) �� ���� ��� ��ȿȭ ***/
	SPARE_reads(flashmem, PBN, PBN_block_meta_buffer_array);

	PBN_block_meta_buffer_array[0]->set_block_state(BLOCK_STATE::NORMAL_BLOCK_INVALID);

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		if ((PBN * BLOCK_PER_SECTOR) + offset_index == PSN || PBN_block_meta_buffer_array[offset_index]->get_sector_state() == SECTOR_STATE::EMPTY) //Overwrite�� ���� ��ġ �Ǵ� �� ��ġ
		{
			//�������� ���߱� ���Ͽ� ��� ������ ���ۿ� �� �������� ���
			block_read_buffer[offset_index] = NULL;
		}
		else
			Flash_read(flashmem, PBN_block_meta_buffer_array[offset_index], (PBN * BLOCK_PER_SECTOR) + offset_index, block_read_buffer[offset_index]);

		/*** meta ���� ���� : ��� �� �ش� ��� ���� ��ȿ ���� ��ȿȭ ***/
		switch (PBN_block_meta_buffer_array[offset_index]->get_sector_state())
		{
		default:
			break;

		case SECTOR_STATE::VALID:
			PBN_block_meta_buffer_array[offset_index]->set_sector_state(SECTOR_STATE::INVALID);
			break;
		}
	}

	SPARE_writes(flashmem, PBN, PBN_block_meta_buffer_array);

	//���� PBN�� ��𿡵� �������� ���� Victim Block���� ����
	update_victim_block_info(flashmem, false, VICTIM_BLOCK_PROC_STATE::UNLINKED, PBN, PBN_block_meta_buffer_array, mapping_method, table_type);

	if (deallocate_block_meta_buffer_array(PBN_block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	switch (table_type)
	{
	case TABLE_TYPE::STATIC:
		break;

	case TABLE_TYPE::DYNAMIC:
		/*** 
			Empty Block ��⿭�� ���� �� �Ϲ� ���� ��� �Ҵ� ���� �� �ش� ��� ���
			�Ҵ� �Ұ� �� Spare Block�� ���
		***/

		if (flashmem->empty_block_queue->dequeue(empty_normal_block_num) == SUCCESS)
		{
			flashmem->block_level_mapping_table[LBN] = empty_normal_block_num;

			SPARE_reads(flashmem, empty_normal_block_num, PBN_block_meta_buffer_array);

			PBN_block_meta_buffer_array[0]->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID);

			for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
			{
				if (block_read_buffer[offset_index] != NULL) //��� ������ �о���� ���ۿ��� �ش� ��ġ�� ������� ������, �� ��ȿ�� �������̸�
				{
					if (Flash_write(flashmem, PBN_block_meta_buffer_array[offset_index], (empty_normal_block_num * BLOCK_PER_SECTOR) + offset_index, block_read_buffer[offset_index]) == COMPLETE)
						goto OVERWRITE_ERR;
				}
				else if (offset_index == Poffset) //��� ���� ���۰� ����ְ�, ��� �� ��ġ�� Overwrite �� ��ġ�� ���ο� �����ͷ� ���
				{
					if (Flash_write(flashmem, PBN_block_meta_buffer_array[offset_index], (empty_normal_block_num * BLOCK_PER_SECTOR) + offset_index, src_data) == COMPLETE)
						goto OVERWRITE_ERR;
				}
				else //����ִ� ��ġ
				{
					//do nothing
				}
			}

			if (deallocate_block_meta_buffer_array(PBN_block_meta_buffer_array) != SUCCESS)
				goto MEM_LEAK_ERR;

			goto END_SUCCESS;
		}

		break;
	}

	/*** Static Table Ȥ�� �� ��� ��⿭���� �� ����� �������� ���� �� : �� Spare ����� ã�Ƽ� ��� ***/
	if (flashmem->spare_block_queue->dequeue(flashmem, empty_spare_block_num, spare_block_queue_index) == FAIL)
		goto SPARE_BLOCK_EXCEPTION_ERR;

	SPARE_reads(flashmem, empty_spare_block_num, PBN_block_meta_buffer_array);

	PBN_block_meta_buffer_array[0]->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID);

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		if (block_read_buffer[offset_index] != NULL) //��� ������ �о���� ���ۿ��� �ش� ��ġ�� ������� ������, �� ��ȿ�� �������̸�
		{
			if (Flash_write(flashmem, PBN_block_meta_buffer_array[offset_index], (empty_spare_block_num * BLOCK_PER_SECTOR) + offset_index, block_read_buffer[offset_index]) == COMPLETE)
				goto OVERWRITE_ERR;
		}
		else if (offset_index == Poffset) //��� ���� ���۰� ����ְ�, ��� �� ��ġ�� Overwrite �� ��ġ�� ���ο� �����ͷ� ���
		{
			if (Flash_write(flashmem, PBN_block_meta_buffer_array[offset_index], (empty_spare_block_num * BLOCK_PER_SECTOR) + offset_index, src_data) == COMPLETE)
				goto OVERWRITE_ERR;
		}
		else //����ִ� ��ġ
		{
			//do nothing
		}
	}
	if (block_read_buffer[0] == NULL) //������ 0���� ����ִٸ� ��� ������ ������� �ʾ����Ƿ� ��� ������ ���� ����
		SPARE_write(flashmem, (empty_spare_block_num * BLOCK_PER_SECTOR), PBN_block_meta_buffer_array[0]);

	if (deallocate_block_meta_buffer_array(PBN_block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	//��� ���� ���̺�� Spare Block ���̺� �󿡼� SWAP
	SWAP(flashmem->block_level_mapping_table[LBN], flashmem->spare_block_queue->queue_array[spare_block_queue_index], tmp);

	goto END_SUCCESS;

HYBRID_LOG_DYNAMIC: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
	/***
		���� �÷��� �޸��� ��ü ũ�⿡ ���Ͽ� ��ü Data Block�� ������ŭ ��� �� �� �־�� �Ѵ�.
		����, ��� LBN�� Data Block, Log Block ��� �Ҵ� ���� ���� ���¿� ���ؼ�
		���ο� �� ���� ���(Data Block)�� �Ҵ� �� �� ���ٸ�, ��ü ��Ͽ� ���� ������ ��Ͽ� Merge�� �����ϰ�, 
		��� LBN�� Log Block�� �Ҵ�Ǿ� �ִ� ���¿� ���ؼ� ���ο� �� ���� ���(Data Block)�� �Ҵ� �� �� ���ٸ�,
		Spare Block�� �̿�, ��ȿ ������ ���� �� ��� ����ó�� ���ο� �����͸� ����Ѵ�.
	***/
	LBN = LSN / BLOCK_PER_SECTOR; //�ش� �� ���Ͱ� ��ġ�ϰ� �ִ� �� ���
	PBN1 = flashmem->log_block_level_mapping_table[LBN][0]; //������ ����� ���� ��� ��ȣ(PBN1)
	PBN2 = flashmem->log_block_level_mapping_table[LBN][1]; //������ ����� ���� ��� ��ȣ(PBN2)
	is_invalid_block = true;
	PBN1_write_proc = PBN2_write_proc = false;

	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
		goto HYBRID_LOG_DYNAMIC_INIT_PROC;
	else if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
		goto HYBRID_LOG_DYNAMIC_PBN1_ASSIGNED_PROC;
	else if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE && PBN2 != DYNAMIC_MAPPING_INIT_VALUE)
		goto HYBRID_LOG_DYNAMIC_PBN2_ASSIGNED_PROC;
	else //PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 != DYNAMIC_MAPPING_INIT_VALUE
		goto HYBRID_LOG_DYNAMIC_BOTH_ASSIGNED_PROC;

HYBRID_LOG_DYNAMIC_INIT_PROC: //�ʱ� ���� (Data Block���� ���� ó��)
	if (flashmem->empty_block_queue->dequeue(flashmem->log_block_level_mapping_table[LBN][0]) != SUCCESS)
	{
		switch (flag_merge_performed) 
		{
		case true: //������ ��ü ��Ͽ� ���Ͽ� ���� �� ��Ͽ� Merge �����Ͽ�����, ��� ������ ���� ����� �������� ���� ���
			goto END_NO_SPACE; //�� �̻� ���ο� ������ ��� �Ұ�

		case false:
			full_merge(flashmem, mapping_method, table_type); //������ ���� ���� ���� ��ü ��Ͽ� ���Ͽ� ���� �� ��Ͽ� Merge ����
			flag_merge_performed = true;

			goto HYBRID_LOG_DYNAMIC;
		}
	}
	
	//���ο� PBN �Ҵ翡 ���� �� ����
	PBN1 = flashmem->log_block_level_mapping_table[LBN][0];
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31
	PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset;

	if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ���
	{
		SPARE_read(flashmem, PSN, PBN1_meta_buffer);

		switch (PBN1_meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			PBN1_meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID); //�ش� ��Ͽ� ��� ���� �����̹Ƿ�, VALID ���·� ����
			break;

		default:
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}
	else //����� ù ��° ������ Spare ������ ���� �ش� ��� ���� �Ǻ� �� ����
	{
		SPARE_read(flashmem, (PBN1 * BLOCK_PER_SECTOR), PBN1_meta_buffer); //����� ù ��° ������ Spare ������ ����

		switch (PBN1_meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			PBN1_meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID); //�ش� ��Ͽ� ��� ���� �����̹Ƿ�, VALID ���·� ����

			SPARE_write(flashmem, (PBN1 * BLOCK_PER_SECTOR), PBN1_meta_buffer); //���� ������ �ϴ� ��ġ�� �ƴϹǷ�, ��� ���� ��� ����

			if (deallocate_single_meta_buffer(PBN1_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		default:
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}

	if (PBN1_meta_buffer == NULL) //��� �� ��ġ�� meta ������ �ǵ� �� �� ���
		SPARE_read(flashmem, PSN, PBN1_meta_buffer); //��� �� ��ġ

	switch (PBN1_meta_buffer->get_sector_state()) //��� �� ��ġ�� ���°� ��� ���� ��쿡 ���
	{
	case SECTOR_STATE::EMPTY:
		PBN1_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC; //�ش� ��ġ�� ���

	default:
		goto WRONG_ASSIGNED_LBN_ERR;
	}

HYBRID_LOG_DYNAMIC_PBN1_ASSIGNED_PROC: //Data Block�� �Ҵ� ����
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31
	PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset;

	if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ���
	{
		SPARE_read(flashmem, PSN, PBN1_meta_buffer);

		switch (PBN1_meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� ����� ���
			break;

		default:
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}
	else //����� ù ��° ������ Spare ������ ���� �ش� ��� ���� �Ǻ�
	{
		SPARE_read(flashmem, (PBN1 * BLOCK_PER_SECTOR), PBN1_meta_buffer); //����� ù ��° ������ Spare ������ ����

		switch (PBN1_meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� ����� ���
			if (deallocate_single_meta_buffer(PBN1_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		default:
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}

	if (PBN1_meta_buffer == NULL) //��� �� ��ġ�� meta ������ �ǵ� �� �� ���
		SPARE_read(flashmem, PSN, PBN1_meta_buffer); //��� �� ��ġ

	switch (PBN1_meta_buffer->get_sector_state()) //��� �� ��ġ�� ���°� ��� ���� ��쿡 ���, �̹� ��ȿ ������ ���� �� Overwrite ó�� ��ƾ���� �̵�
	{
	case SECTOR_STATE::EMPTY:
		PBN1_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC; //�ش� ��ġ�� ���

	case SECTOR_STATE::VALID: //PBN1�� ���� ������ ��ġ ��ȿȭ
		PBN1_meta_buffer->set_sector_state(SECTOR_STATE::INVALID);

		SPARE_write(flashmem, PSN, PBN1_meta_buffer);

		if (deallocate_single_meta_buffer(PBN1_meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;
		break;

	default:
		goto WRONG_ASSIGNED_LBN_ERR;
	}

	/*** Log Block �Ҵ� �� ���������� �� �����¿� ��� ***/
	if (flashmem->empty_block_queue->dequeue(flashmem->log_block_level_mapping_table[LBN][1]) != SUCCESS) //�Ҵ� ���� �� �� ���� ����� �������� ���� �� Spare Block�� ���� ���� Ovewrwrite ó�� ��ƾ���� �̵�
	{
		PBN_for_overwrite_proc = PBN1;
		PBN1 = flashmem->log_block_level_mapping_table[LBN][0] = DYNAMIC_MAPPING_INIT_VALUE;

		goto HYBRID_LOG_DYNAMIC_COMMON_OVERWRITE_PROC; //�Ҵ� ���� �� �� ���� ����� �������� ���� �� Spare Block�� ���� ���� Ovewrwrite ó�� ��ƾ���� �̵�
	}

	//���ο� PBN2 �Ҵ翡 ���� �� ����
	PBN2 = flashmem->log_block_level_mapping_table[LBN][1];
	offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
	
	if (search_empty_offset_in_block(flashmem, PBN2, flashmem->offset_level_mapping_table[offset_level_table_index], PBN2_meta_buffer, mapping_method) != SUCCESS)
	{
		//�ӽÿ���ó��
		printf("�� �������� ���� ���� ����");
		system("pause");
	}
	
	Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
	PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

	PBN2_meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID);

	if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ���
	{
		PBN2_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC; //�ش� ������ ��ġ�� ���
	}
	else
	{
		SPARE_write(flashmem, (PBN2 * BLOCK_PER_SECTOR), PBN2_meta_buffer); //���� ������ �ϴ� ��ġ�� �ƴϹǷ�, ��� ���� ��� ����

		if (deallocate_single_meta_buffer(PBN2_meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		PBN2_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC; //�ش� ������ ��ġ�� ���
	}

HYBRID_LOG_DYNAMIC_PBN2_ASSIGNED_PROC: //Log Block�� �Ҵ� ����
	Loffset = LSN % BLOCK_PER_SECTOR; //������ ���� ���̺� �������� LSN�� �� ������
	offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
	Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
	PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

	/***
		Data Block���� Ư�� �����¿� ���� �ݺ��� Overwrite�� �߻����� �ʰ�, 
		Data Block���� ��� �����¿� ���� Overwrite�� �߻��Ͽ� Log Block�� �Ҵ� �� ��Ȳ
	***/

	if (Poffset == OFFSET_MAPPING_INIT_VALUE)
		goto WRONG_ASSIGNED_OFFSET_ERR;

	/*** PBN2�� ���� ������ ��ġ ��ȿȭ ***/
	SPARE_read(flashmem, PSN, PBN2_meta_buffer);
	PBN2_meta_buffer->set_sector_state(SECTOR_STATE::INVALID);
	SPARE_write(flashmem, PSN, PBN2_meta_buffer);

	if (deallocate_single_meta_buffer(PBN2_meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;

	flashmem->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE;

	/*** Data Block �Ҵ� �� ���ο� ������ ��� ***/
	if (flashmem->empty_block_queue->dequeue(flashmem->log_block_level_mapping_table[LBN][0]) != SUCCESS)
	{
		PBN_for_overwrite_proc = PBN2;
		PBN2 = flashmem->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE;

		goto HYBRID_LOG_DYNAMIC_COMMON_OVERWRITE_PROC; //�Ҵ� ���� �� �� ���� ����� �������� ���� �� Spare Block�� ���� ���� Ovewrwrite ó�� ��ƾ���� �̵�
	}

	//���ο� PBN �Ҵ翡 ���� �� ����
	PBN1 = flashmem->log_block_level_mapping_table[LBN][0];
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31
	PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset;

	if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ���
	{
		SPARE_read(flashmem, PSN, PBN1_meta_buffer);

		switch (PBN1_meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			PBN1_meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID); //�ش� ��Ͽ� ��� ���� �����̹Ƿ�, VALID ���·� ����
			break;

		default:
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}
	else //����� ù ��° ������ Spare ������ ���� �ش� ��� ���� �Ǻ� �� ����
	{
		SPARE_read(flashmem, (PBN1 * BLOCK_PER_SECTOR), PBN1_meta_buffer); //����� ù ��° ������ Spare ������ ����

		switch (PBN1_meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_EMPTY: //�� ����� ���
			PBN1_meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID); //�ش� ��Ͽ� ��� ���� �����̹Ƿ�, VALID ���·� ����

			SPARE_write(flashmem, (PBN * BLOCK_PER_SECTOR), PBN1_meta_buffer); //���� ������ �ϴ� ��ġ�� �ƴϹǷ�, ��� ���� ��� ����

			if (deallocate_single_meta_buffer(PBN1_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;
			break;

		default:
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}

	if (PBN1_meta_buffer == NULL) //��� �� ��ġ�� meta ������ �ǵ� �� �� ���
		SPARE_read(flashmem, PSN, PBN1_meta_buffer); //��� �� ��ġ

	switch (PBN1_meta_buffer->get_sector_state()) //��� �� ��ġ�� ���°� ��� ���� ��쿡 ���
	{
	case SECTOR_STATE::EMPTY:
		PBN1_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC; //�ش� ��ġ�� ���

	default:
		goto WRONG_ASSIGNED_LBN_ERR;
	}

HYBRID_LOG_DYNAMIC_BOTH_ASSIGNED_PROC: //Data Block, Log Block ��� �Ҵ� ����
	//Data Block���� ���� �Ǻ��Ͽ� ó��
	Loffset = Poffset = LSN % BLOCK_PER_SECTOR; //��� ���� �� ���� offset = 0 ~ 31
	PSN = (PBN1 * BLOCK_PER_SECTOR) + Poffset;

	if (PSN % BLOCK_PER_SECTOR == 0) //��� �� ��ġ�� ����� ù ��° ������ ���
	{
		SPARE_read(flashmem, PSN, PBN1_meta_buffer);

		switch (PBN1_meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� ����� ���
			break;

		default:
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}
	else //����� ù ��° ������ Spare ������ ���� �ش� ��� ���� �Ǻ�
	{
		SPARE_read(flashmem, (PBN1 * BLOCK_PER_SECTOR), PBN1_meta_buffer); //����� ù ��° ������ Spare ������ ����

		switch (PBN1_meta_buffer->get_block_state())
		{
		case BLOCK_STATE::NORMAL_BLOCK_VALID: //��ȿ�� ����� ���
			if (deallocate_single_meta_buffer(PBN1_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			break;

		default:
			goto WRONG_ASSIGNED_LBN_ERR;
		}
	}

	if (PBN1_meta_buffer == NULL) //��� �� ��ġ�� meta ������ �ǵ� �� �� ���
		SPARE_read(flashmem, PSN, PBN1_meta_buffer); //��� �� ��ġ

	switch (PBN1_meta_buffer->get_sector_state())
	{
	case SECTOR_STATE::EMPTY:
		/***
			�ش� �����¿� ���Ͽ� Log Block���� ��� ���θ� �Ǻ�, ������ �ʾ��� ��� Data Block�� ���, ���Ǿ��� ��� Log Block�� ���� ������ ��ȿȭ
			Log Block�� �Ҵ�Ǿ� �ִ� ��Ȳ���� Data Block�� �Ҵ� �� ��� Merge ������ ���̱� ���� Data Block�� ���� �����Ϳ� ���Ͽ� 1ȸ�� Overwrite�� ������ Log Blockó�� ���
		***/

		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��

		if (flashmem->offset_level_mapping_table[offset_level_table_index] == OFFSET_MAPPING_INIT_VALUE) //Log Block���� �ش� �����¿� ���� ������ �ʾ��� ��� Data Block�� ���
		{
			PBN1_write_proc = true;
			goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC; //�ش� ��ġ�� ���
		}

		/*** Log Block�� ���� ������ ��ġ ��ȿȭ ***/
		SPARE_reads(flashmem, PBN2, PBN2_block_meta_buffer_array);

		PBN2_block_meta_buffer_array[flashmem->offset_level_mapping_table[offset_level_table_index]]->set_sector_state(SECTOR_STATE::INVALID);

		/*** �̿� ����, ����, Log Block�� ��� �����Ͱ� ��ȿȭ�Ǿ�����, Log Block ��ȿȭ ***/
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			if (PBN2_block_meta_buffer_array[offset_index]->get_sector_state() != SECTOR_STATE::INVALID) //��ȿ�ϰų�, ���������
			{
				is_invalid_block = false;
				break;
			}
		}
		if (is_invalid_block) //Log Block�� ��� �����Ͱ� ��ȿȭ�Ǿ�����
		{
			PBN2_block_meta_buffer_array[0]->set_block_state(BLOCK_STATE::NORMAL_BLOCK_INVALID);
			flashmem->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE; //PBN2�� ��� ���� ���� ���̺� �󿡼� Unlink(���� ����)

			//if (flashmem->offset_level_mapping_table[offset_level_table_index] % BLOCK_PER_SECTOR == 0) //���� ������ ��ġ�� 0��° ������ ��ġ�� ��ȿȭ�� ��ϰ� ���� ���� ���� ����
			if (flashmem->offset_level_mapping_table[offset_level_table_index] == 0) //���� ������ ��ġ�� 0��° ������ ��ġ�� ��ȿȭ�� ��ϰ� ���� ���� ���� ����
			{
				SPARE_write(flashmem, (PBN2 * BLOCK_PER_SECTOR), PBN2_block_meta_buffer_array[0]); //���, ��ȿȭ�� ���� ���� ����
			}
			else
			{
				SPARE_write(flashmem, (PBN2 * BLOCK_PER_SECTOR), PBN2_block_meta_buffer_array[0]); //��� ���� ����
				SPARE_write(flashmem, (PBN2 * BLOCK_PER_SECTOR) + (flashmem->offset_level_mapping_table[offset_level_table_index]), PBN2_block_meta_buffer_array[flashmem->offset_level_mapping_table[offset_level_table_index]]); //��ȿȭ�� ���� ���� ����
			}

			//���� Log Block�� ��𿡵� �������� ���� Victim Block���� ����
			update_victim_block_info(flashmem, false, VICTIM_BLOCK_PROC_STATE::UNLINKED, PBN2, PBN2_block_meta_buffer_array, mapping_method, table_type);
			PBN2 = DYNAMIC_MAPPING_INIT_VALUE;
		}
		else //��ȿȭ�� ���� ������ ����
			SPARE_write(flashmem, (PBN2 * BLOCK_PER_SECTOR) + (flashmem->offset_level_mapping_table[offset_level_table_index]), PBN2_block_meta_buffer_array[flashmem->offset_level_mapping_table[offset_level_table_index]]); //��ȿȭ�� ���� ���� ����

		flashmem->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE;
		
		if (deallocate_block_meta_buffer_array(PBN2_block_meta_buffer_array) != SUCCESS)
			goto MEM_LEAK_ERR;

		/*** Data Block�� �ش� ������ ��ġ�� ��� ***/
		PBN1_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC; //�ش� ��ġ�� ���

	case SECTOR_STATE::INVALID:
		/***
			��ȿ�� �����Ͱ� Log Block�� ��ϵǾ� �����Ƿ�, Log Block�� ���� ������ ��ġ ��ȿȭ
			---
			Log Block�� ��� ������ ��ȿȭ ���θ� �Ǻ��Ͽ� Log Block�� ��ȿȭ �� Unlink �����Ͽ��� �ϴ°�?
			=> Log Block�� �� ���� �������� �����ϴ��� Ž���ϴ� �������� �� ���� �������� �������� ������
			Data Block�� Log Block�� Merge �����ϹǷ�, Log Block�� ��� ������ ��ȿȭ ���δ� ���� �Ǻ����� �ʴ´�.
		***/
		if (deallocate_single_meta_buffer(PBN1_meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��
		Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
		PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

		SPARE_read(flashmem, PSN, PBN2_meta_buffer);
		PBN2_meta_buffer->set_sector_state(SECTOR_STATE::INVALID);
		SPARE_write(flashmem, PSN, PBN2_meta_buffer);

		if (deallocate_single_meta_buffer(PBN2_meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		/*** Log Block�� ���������� �� ���� ������ �Ǻ� : ���ο� �����͸� Log Block�� ���������� �� ���� �����¿� ��� ***/
		if (search_empty_offset_in_block(flashmem, PBN2, flashmem->offset_level_mapping_table[offset_level_table_index], PBN2_meta_buffer, mapping_method) != SUCCESS)
		{
			/*** Log Block�� �� ���� �������� �������� ���� �� Data Block, Log Block�� ������ �� Spare Block�� ����Ͽ� ��ȿ ������ Merge ���� ***/
			flashmem->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE;
			full_merge(flashmem, LBN, mapping_method, table_type);
			goto HYBRID_LOG_DYNAMIC; //���ο� ������ ����� ���� �� ����
		}

		//���ο� ������ �Ҵ翡 ���� �� ����
		Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
		PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

		/*** Log Block�� ���������� �� �ش� ������ ��ġ�� ��� ***/
		PBN2_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC;

		break;

	case SECTOR_STATE::VALID:
		/***
			1) Data Block�� �ش� ��ġ ��ȿȭ, �̿� ���� Data Block�� ��� ������ ��ȿȭ ���� �Ǻ��Ͽ�
			��� ��ȿȭ �� Data Block ��ȿȭ, Unlink, Data Block Erase ���� ���� Victim Block���� ����

			2) Log Block�� �� ���� �������� ���� �� ��� �ش� ��ġ�� ���ο� ������ ���

			3) Log Block�� �� ���� �������� ���� ���� ���� ��� ���� Data Block �Ҵ� ���ο� ���� ó��
			- Data Block �Ҵ� �� : Data Block, Log Block�� ������ �� Spare Block�� ����Ͽ� ��ȿ ������ Merge ���� �� ���ο� ������ ���
			- Data Block ��ȿȭ�Ǿ� �Ҵ���� �ʾ��� �� : Log Block ����� ��ġ�� ������ ���͵��� �����͸� �� Spare Block���� ���� �� ���ο� ������ ���
		***/

		if (deallocate_single_meta_buffer(PBN1_meta_buffer) != SUCCESS)
			goto MEM_LEAK_ERR;

		/*** Data Block�� ���� ������ ��ġ ��ȿȭ ***/
		SPARE_reads(flashmem, PBN1, PBN1_block_meta_buffer_array);

		PBN1_block_meta_buffer_array[Poffset]->set_sector_state(SECTOR_STATE::INVALID);

		/*** �̿� ����, ����, Data Block�� ��� �����Ͱ� ��ȿȭ�Ǿ�����, Data Block ��ȿȭ ***/
		for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
		{
			if (PBN1_block_meta_buffer_array[offset_index]->get_sector_state() != SECTOR_STATE::INVALID) //��ȿ�ϰų�, ���������
			{
				is_invalid_block = false;
				break;
			}
		}
		if (is_invalid_block) //Data Block�� ��� �����Ͱ� ��ȿȭ�Ǿ�����
		{
			PBN1_block_meta_buffer_array[0]->set_block_state(BLOCK_STATE::NORMAL_BLOCK_INVALID);
			flashmem->log_block_level_mapping_table[LBN][0] = DYNAMIC_MAPPING_INIT_VALUE; //PBN1�� ��� ���� ���� ���̺� �󿡼� Unlink(���� ����)
			
			if (Poffset == 0) //���� ������ ��ġ�� 0��° ������ ��ġ�� ��ȿȭ�� ��ϰ� ���� ���� ���� ����
			{
				SPARE_write(flashmem, (PBN1 * BLOCK_PER_SECTOR), PBN1_block_meta_buffer_array[0]); //���, ��ȿȭ�� ���� ���� ����
			}
			else
			{
				SPARE_write(flashmem, (PBN1 * BLOCK_PER_SECTOR), PBN1_block_meta_buffer_array[0]); //��� ���� ����
				SPARE_write(flashmem, (PBN1 * BLOCK_PER_SECTOR) + Poffset, PBN1_block_meta_buffer_array[Poffset]); //��ȿȭ�� ���� ���� ����
			}

			//���� Data Block�� ��𿡵� �������� ���� Victim Block���� ����
			update_victim_block_info(flashmem, false, VICTIM_BLOCK_PROC_STATE::UNLINKED, PBN1, PBN1_block_meta_buffer_array, mapping_method, table_type);
			PBN1 = DYNAMIC_MAPPING_INIT_VALUE;
		}
		else //��ȿȭ�� ���� ������ ����
			SPARE_write(flashmem, (PBN1 * BLOCK_PER_SECTOR) + Poffset, PBN1_block_meta_buffer_array[Poffset]); //��ȿȭ�� ���� ���� ����

		if (deallocate_block_meta_buffer_array(PBN1_block_meta_buffer_array) != SUCCESS)
			goto MEM_LEAK_ERR;

		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + Loffset; //������ ���� ���̺� �������� �ش� LSN�� index��

		/*** Log Block�� ���������� �� ���� ������ �Ǻ� : ���ο� �����͸� Log Block�� ���������� �� ���� �����¿� ��� ***/
		if (search_empty_offset_in_block(flashmem, PBN2, flashmem->offset_level_mapping_table[offset_level_table_index], PBN2_meta_buffer, mapping_method) != SUCCESS)
		{
			//Data Block �Ҵ� ���ο� ����
			if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE) //Data Block �Ҵ� �� : Data Block, Log Block�� ������ �� Spare Block�� ����Ͽ� ��ȿ ������ Merge ���� �� ���ο� ������ ���
			{
				full_merge(flashmem, LBN, mapping_method, table_type);
				goto HYBRID_LOG_DYNAMIC; //���ο� ������ ����� ���� �� ����
			}
			else //Data Block ��ȿȭ�Ǿ� �Ҵ���� �ʾ��� �� : Log Block ����� ��ġ�� ������ ���͵��� �����͸� �� Spare Block���� ���� �� ���ο� ������ ���
			{
				//����, Log Block���� ��� �����Ͱ� Data Block�� Sequence�� �����ϰ� ��ϵǾ� �ִٸ�, �ܼ� Switch Merge�����ε� ����
				//Log Block ���� ��� Sequence Ȯ�� �������� �ϴܺ���
				///
				/// 

				PBN_for_overwrite_proc = PBN2;
				PBN2 = flashmem->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE; //PBN2�� ��� ���� ���� ���̺� �󿡼� Unlink(���� ����)

				goto HYBRID_LOG_DYNAMIC_COMMON_OVERWRITE_PROC; //Spare Block�� ���� ���� Ovewrwrite ó�� ��ƾ���� �̵�
			}
		}

		//���ο� ������ �Ҵ翡 ���� �� ����
		Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //LSN�� �ش��ϴ� ���� ��� ���� ������ ������ ��ġ
		PSN = (PBN2 * BLOCK_PER_SECTOR) + Poffset;

		/*** Log Block�� ���������� �� �ش� ������ ��ġ�� ��� ***/
		PBN2_write_proc = true;
		goto HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC;

		break;
	}

HYBRID_LOG_DYNAMIC_COMMON_OVERWRITE_PROC: //���̺긮�� ���� ���� Overwrite ó�� ��ƾ : �Ҵ� ���� �� �� ���� ����� �������� �ʰų�, Data Block ��ȿȭ�� ���� Log Block�� ��ȿ �����͵鿡 ���� Spare Block�� ���� ó��
	/***
		1) ���� ����� ��ȿ ������ �� ���ο� �����͸� Spare Block�� ���
		2) ���� ��ϰ� �ش� Spare Block ��ü
		3) ���� ��� Erase ���� Victim Block���� ���� (Unlinked)
		4) �ش� Spare Block�� Data Block���� �Ҵ�
	***/

	/*** ��ȿ ������ ���� (Overwrite�� ��ġ �� �� ��ġ�� ����) �� ���� ��� ��ȿȭ ***/
	SPARE_reads(flashmem, PBN_for_overwrite_proc, PBN_block_meta_buffer_array);

	PBN_block_meta_buffer_array[0]->set_block_state(BLOCK_STATE::NORMAL_BLOCK_INVALID);

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		if ((PBN_for_overwrite_proc * BLOCK_PER_SECTOR) + offset_index == PSN || PBN_block_meta_buffer_array[offset_index]->get_sector_state() == SECTOR_STATE::EMPTY) //Overwrite�� ���� ��ġ �Ǵ� �� ��ġ
		{
			//�������� ���߱� ���Ͽ� ��� ������ ���ۿ� �� �������� ���
			block_read_buffer[offset_index] = NULL;
		}
		else
			Flash_read(flashmem, PBN_block_meta_buffer_array[offset_index], (PBN_for_overwrite_proc * BLOCK_PER_SECTOR) + offset_index, block_read_buffer[offset_index]);

		/*** meta ���� ���� : ��� �� �ش� ��� ���� ��ȿ ���� ��ȿȭ ***/
		switch (PBN_block_meta_buffer_array[offset_index]->get_sector_state())
		{
		default:
			break;

		case SECTOR_STATE::VALID:
			PBN_block_meta_buffer_array[offset_index]->set_sector_state(SECTOR_STATE::INVALID);
			break;
		}
	}

	SPARE_writes(flashmem, PBN_for_overwrite_proc, PBN_block_meta_buffer_array);

	//���� PBN�� Spare Block ��⿭�� ������ Victim Block���� ����
	update_victim_block_info(flashmem, false, VICTIM_BLOCK_PROC_STATE::SPARE_LINKED, PBN_for_overwrite_proc, PBN_block_meta_buffer_array, mapping_method, table_type);

	if (deallocate_block_meta_buffer_array(PBN_block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	/*** �Ҵ� ������ �� ���� ����� �������� �����Ƿ�, Spare Block�� ����Ͽ� ��ȿ ������ �� ���ο� ������ ��� ***/
	if (flashmem->spare_block_queue->dequeue(flashmem, empty_spare_block_num, spare_block_queue_index) == FAIL)
		goto SPARE_BLOCK_EXCEPTION_ERR;

	SPARE_reads(flashmem, empty_spare_block_num, PBN_block_meta_buffer_array);

	PBN_block_meta_buffer_array[0]->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID);

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		if (block_read_buffer[offset_index] != NULL) //��� ������ �о���� ���ۿ��� �ش� ��ġ�� ������� ������, �� ��ȿ�� �������̸�
		{
			if (Flash_write(flashmem, PBN_block_meta_buffer_array[offset_index], (empty_spare_block_num * BLOCK_PER_SECTOR) + offset_index, block_read_buffer[offset_index]) == COMPLETE)
				goto OVERWRITE_ERR;
		}
		else if (offset_index == Poffset) //��� ���� ���۰� ����ְ�, ��� �� ��ġ�� Overwrite �� ��ġ�� ���ο� �����ͷ� ���
		{
			if (Flash_write(flashmem, PBN_block_meta_buffer_array[offset_index], (empty_spare_block_num * BLOCK_PER_SECTOR) + offset_index, src_data) == COMPLETE)
				goto OVERWRITE_ERR;
		}
		else //����ִ� ��ġ
		{
			//do nothing
		}
	}
	if (block_read_buffer[0] == NULL) //������ 0���� ����ִٸ� ��� ������ ������� �ʾ����Ƿ� ��� ������ ���� ����
		SPARE_write(flashmem, (empty_spare_block_num * BLOCK_PER_SECTOR), PBN_block_meta_buffer_array[0]);

	if (deallocate_block_meta_buffer_array(PBN_block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	/*** �ش� Spare Block�� Data Block���� �Ҵ�, ���� Spare Block ��ġ�� ���� ��� ���� ***/
	flashmem->log_block_level_mapping_table[LBN][0] = empty_spare_block_num; //DYNAMIC_MAPPING_INIT_VALUE�� �ʱ�ȭ �Ǿ����Ƿ�, �ܼ� ����
	flashmem->spare_block_queue->queue_array[spare_block_queue_index] = PBN_for_overwrite_proc;

	goto END_SUCCESS;

HYBRID_LOG_DYNAMIC_COMMON_WRITE_PROC: //���̺긮�� ���� ���� ��� ó�� ��ƾ
	if (PBN1_write_proc == true && PBN2_write_proc == false) //Data Block (PBN1)�� ���� ���
	{
		PBN1_write_proc = false;

		if ((PBN1_meta_buffer == NULL && PBN1_block_meta_buffer_array == NULL) || (PBN1_meta_buffer != NULL && PBN1_block_meta_buffer_array == NULL))
		{
			if(PBN1_meta_buffer == NULL)
				SPARE_read(flashmem, PSN, PBN1_meta_buffer);

			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, PBN1_meta_buffer, PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;

			if (deallocate_single_meta_buffer(PBN1_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

		}
		else if (PBN1_meta_buffer == NULL && PBN1_block_meta_buffer_array != NULL)
		{
			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, PBN1_block_meta_buffer_array[Poffset], PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;

			if (deallocate_block_meta_buffer_array(PBN1_block_meta_buffer_array) != SUCCESS)
				goto MEM_LEAK_ERR;
		}
		else //PBN1_meta_buffer != NULL && PBN1_block_meta_buffer_array != NULL
			goto MEM_LEAK_ERR;
	}
	else if (PBN1_write_proc == false && PBN2_write_proc == true) //Log Block (PBN2)�� ���� ���
	{
		PBN2_write_proc = false;
		
		if ((PBN2_meta_buffer == NULL && PBN2_block_meta_buffer_array == NULL) || (PBN2_meta_buffer != NULL && PBN2_block_meta_buffer_array == NULL))
		{
			if (PBN2_meta_buffer == NULL)
				SPARE_read(flashmem, PSN, PBN2_meta_buffer);

			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, PBN2_meta_buffer, PSN, src_data) == COMPLETE)
				goto OVERWRITE_ERR;

			if (deallocate_single_meta_buffer(PBN2_meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

		}
		else if (PBN2_meta_buffer == NULL && PBN2_block_meta_buffer_array != NULL)
		{
			//�ش� ������ ��ġ�� ���
			if (Flash_write(flashmem, PBN2_block_meta_buffer_array[Poffset], PSN, src_data) == COMPLETE)
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
	if (meta_buffer != NULL || PBN_block_meta_buffer_array != NULL || PBN1_meta_buffer != NULL || PBN2_meta_buffer != NULL || PBN1_block_meta_buffer_array != NULL || PBN2_block_meta_buffer_array != NULL)
		goto MEM_LEAK_ERR;
	
	switch (flashmem->v_flash_info.flash_state)
	{
	case FLASH_STATE::WRITE:
		flashmem->v_flash_info.flash_state = FLASH_STATE::IDLE; //���� �������� �˸�

	default:
		break;
	}

	if(mapping_method == MAPPING_METHOD::HYBRID_LOG)
		if (PBN1 != DYNAMIC_MAPPING_INIT_VALUE && PBN2 != DYNAMIC_MAPPING_INIT_VALUE)
			update_victim_block_info(flashmem, true, VICTIM_BLOCK_PROC_STATE::UNPROCESSED, LBN, mapping_method, table_type); //��ȿ�� �Ӱ谪�� ���� Victim Block ����

	flashmem->save_table(mapping_method);

	return SUCCESS;

END_NO_SPACE: //��� ���� ���� ����
	fprintf(stderr, "��� �� ������ �������� ���� : �Ҵ�� ũ���� ���� ��� ���\n");
	return FAIL;

SPARE_BLOCK_EXCEPTION_ERR: //Spare Block Queue�� ���� ����
	if(VICTIM_BLOCK_QUEUE_RATIO != SPARE_BLOCK_RATIO)
		fprintf(stderr, "Spare Block Queue�� �Ҵ�� ũ���� ���� ��� ��� : �̱���, GC�� ���� ó���ǵ��� �ؾ��Ѵ�.\n");
	else
	{
		fprintf(stderr, "ġ���� ���� : Spare Block Queue �� GC Scheduler�� ���� ���� �߻� (FTL_write)\n");
		flashmem->victim_block_queue->print();
		flashmem->spare_block_queue->print();
		system("pause");
		exit(1);
	}

	return FAIL;

WRONG_MAPPING_METHOD_ERR:
	fprintf(stderr, "ġ���� ���� : �߸��� ���� ��� (FTL_write)\n");
	system("pause");
	exit(1);

WRONG_TABLE_TYPE_ERR:
	fprintf(stderr, "ġ���� ���� : �߸��� ���̺� Ÿ�� (FTL_write)\n");
	system("pause");
	exit(1);

INVALID_PAGE_ERR:
	fprintf(stderr, "ġ���� ���� : Invalid Page (FTL_write)");
	system("pause");
	exit(1);

INVALID_BLOCK_ERR:
	fprintf(stderr, "ġ���� ���� : Invalid Block (FTL_write)");
	system("pause");
	exit(1);

WRONG_ASSIGNED_LBN_ERR:
	fprintf(stderr, "ġ���� ���� : �߸� �Ҵ� �� LBN (FTL_write)\n");
	system("pause");
	exit(1);

WRONG_STATIC_TABLE_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ���̺� ���� �������� ���� ���̺� (FTL_write\n");
	system("pause");
	exit(1);

WRONG_ASSIGNED_OFFSET_ERR:
	fprintf(stderr, "ġ���� ���� : �߸� �Ҵ� �� offset (FTL_write)\n");
	system("pause");
	exit(1);

WRITE_COND_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : ���� ���� ���� (FTL_write\n");
	system("pause");
	exit(1);
	
MERGE_COND_EXCEPTION_ERR:
	fprintf(stderr, "ġ���� ���� : PBN 1 : %u , PBN2 : %u Merge ���� ���� (FTL_write)\n", PBN1, PBN2);
	system("pause");
	exit(1);

OVERWRITE_ERR:
	fprintf(stderr, "ġ���� ���� : Overwrite�� ���� ���� �߻� (FTL_write)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (FTL_write)\n");
	system("pause");
	exit(1);
}

int full_merge(FlashMem*& flashmem, unsigned int LBN, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //Ư�� LBN�� ������ PBN1�� PBN2�� ���Ͽ� Merge ����
{
	switch (mapping_method) //���� ����
	{
	case MAPPING_METHOD::HYBRID_LOG:
		break;

	default:
		return FAIL;
	}

	unsigned int PBN1 = flashmem->log_block_level_mapping_table[LBN][0]; //LBN�� ������ ���� ���(PBN1)
	unsigned int PBN2 = flashmem->log_block_level_mapping_table[LBN][1]; //LBN�� ������ ���� ���(PBN2)

	__int8 Loffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ���� ���� Offset = 0 ~ 31
	__int8 Poffset = OFFSET_MAPPING_INIT_VALUE; //��� ���� ������ ���� Offset = 0 ~ 31
	unsigned int offset_level_table_index = DYNAMIC_MAPPING_INIT_VALUE; //������ ���� ���̺� �ε���

	unsigned int empty_spare_block_num = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare ���
	unsigned int spare_block_queue_index = DYNAMIC_MAPPING_INIT_VALUE; //����� �� Spare ����� Spare Block Queue �� �ε���
	unsigned int tmp = DYNAMIC_MAPPING_INIT_VALUE; //���̺� SWAP���� �ӽ� ����

	char read_buffer = NULL; //���� ���ͷκ��� �о���� ������
	char block_read_buffer[BLOCK_PER_SECTOR] = { NULL }; //�� ��� ���� ������ �ӽ� ���� ����
	__int8 read_buffer_index = 0; //�����͸� �о �ӽ������ϱ� ���� read_buffer �ε��� ����

	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����
	META_DATA** PBN1_block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� meta ���� (PBN1)
	META_DATA** PBN2_block_meta_buffer_array = NULL; //�� ���� ��� ���� ��� ����(������)�� ���� meta ���� (PBN2)

	f_flash_info = flashmem->get_f_flash_info();

	/***
		Full Merge ���� :
		- Data Block(PBN1)�� Log Block(PBN2)�� ��ȿ�� ��(�ʱⰪ�� �ƴ� ������ ��)�̾�� ��
		- Data Block(PBN1)�� Log Block(PBN2)�� Spare Block�� �ƴ� ��
		
		��� �����Ϳ� ���� ����� �� �� ���� PBN1�� ��ϵǰ�, Overwrite�ÿ� Log Block(PBN2)�� ��ϵǹǷ�
		- Data Block(PBN1)���� � �� �������� ��ȿ�� ��(INVALID)�̶�� Log Block(PBN2)���� �ش� �� �����¿� ��ȿ�� �����Ͱ� �׻� ����
		- Data Block(PBN1)���� � �� �����¿� ���� ��ȿ�� ��(VALID)�̶�� Log Block(PBN2)���� �ش� �� �����¿� ���� �׼� ��ϵǾ� ���� ����
		=> Overwrite�ÿ� Log Block(PBN2)�� ����ϹǷ�, Data Block(PBN1)�� � �� �������� ��ȿ�ϴٸ� Log Block(PBN2)�� ���� �� �������� ������� �� �ۿ� ����
	
		Data Block(PBN1)�� Log Block(PBN2)�κ��� �� �����¿� ���� ��ȿ �����͵��� ���۷� ����
		Data Block(PBN1)�� �� ������ ��ġ�� ��ȿ�� ��� Log Block(PBN2)���� �ش� �� ������ ��ġ�� �д´�
		=> PBN1�� �� �Ҵ�
		
		---
		< ���� ���� >
		������ ��� ������ ��� LBN�� ���� Data Block, Log Block ��� �Ҵ� �Ǿ� �ִ� ��Ȳ����
		Data Block�� ��� ������ ��ġ�� ��ȿ�ϰ�, Log Block���� �ش� �� �����¿� �ش��ϴ� ��ȿ�� ���� ������ ��ġ�� ��ȿȭ ��Ų�ٸ� 
		Full Merge�������� 'Data Block(PBN1)���� � �� �������� ��ȿ�� ��(INVALID)�̶�� Log Block(PBN2)���� �ش� �� �����¿� ��ȿ�� �����Ͱ� �׻� ����'�� �����Ѵ�. 
		����, Data Block���� � �� �������� ��ȿ�ϸ� Log Block�� �ش� ���� �������� �˻��Ѵ�.
		����, Log Block�� ���� �������� ���� �Ǿ� ���� ������, Full Merge������ ���� �� ���ο� ������ ����� ���� �ش� ������ ��ġ�� NULL�� ó��
	***/

	//Merge�� ���� PBN1�� PBN2�� ���� �� �����Ǿ� �־�� ��
	if (PBN1 == DYNAMIC_MAPPING_INIT_VALUE || PBN2 == DYNAMIC_MAPPING_INIT_VALUE)
		return COMPLETE; //�ش� LBN�� ���Ͽ� Merge �Ұ�

	printf("Performing Merge on LBN : %u (PBN1 : %u, PBN2 : %u)...\n", LBN, PBN1, PBN2);

	/***
		PBN1�� PBN2�κ��� �� �����¿� ���� ��ȿ �����͵��� ���۷� ����
		1) PBN1�� �� ������ ��ġ�� ��ȿ�� ���
		2) PBN2���� �ش� �� ������ ��ġ�� �д´�
	***/

	SPARE_reads(flashmem, PBN1, PBN1_block_meta_buffer_array);
	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR;  offset_index++)
	{
		if (PBN1_block_meta_buffer_array[offset_index]->get_sector_state() == SECTOR_STATE::VALID)
		{
			//PBN1���� ������� �ʰ�, ��ȿ�ϸ� (PBN1 : Loffset == Poffset)
			Flash_read(flashmem, PBN1_block_meta_buffer_array[offset_index], (PBN1 * BLOCK_PER_SECTOR) + offset_index, block_read_buffer[offset_index]);
		}
		else if (PBN1_block_meta_buffer_array[offset_index]->get_sector_state() == SECTOR_STATE::EMPTY)
		{
			//PBN1���� ����ְ�, ��ȿ�ϸ� �������� ���߱����� �� �������� ��� (��, �ѹ��� ��ϵ��� ���� ��ġ)
			block_read_buffer[offset_index] = NULL;
		}
		else //PBN1���� ��ȿ���� ������
		{
			offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + offset_index; //PBN2�� ������ ���� ���̺� �������� PBN1������ Loffset�� �ش��ϴ� index��
			Poffset = flashmem->offset_level_mapping_table[offset_level_table_index]; //���� ��� ���� ������ ������ ��ġ

			if (Poffset != OFFSET_MAPPING_INIT_VALUE) //PBN2�� ���� �������� �Ҵ�Ǿ� ���� ��� PBN2���� �ش� �� ������ ��ġ�� �д´�
				Flash_read(flashmem, DO_NOT_READ_META_DATA, (PBN2 * BLOCK_PER_SECTOR) + Poffset, block_read_buffer[offset_index]);
			else
				block_read_buffer[offset_index] = NULL; //���ο� ������ ����� ���� �� �������� ���
		}
	}

	if (deallocate_block_meta_buffer_array(PBN1_block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	/*** PBN1, PBN2 Erase ***/
	Flash_erase(flashmem, PBN1);
	Flash_erase(flashmem, PBN2);

	/*** �� Spare ����� ã�Ƽ� ��� ***/
	if (flashmem->spare_block_queue->dequeue(flashmem, empty_spare_block_num, spare_block_queue_index) == FAIL)
		goto SPARE_BLOCK_EXCEPTION_ERR;

	SPARE_reads(flashmem, empty_spare_block_num, PBN1_block_meta_buffer_array);

	PBN1_block_meta_buffer_array[0]->set_block_state(BLOCK_STATE::NORMAL_BLOCK_VALID);

	for (__int8 offset_index = 0; offset_index < BLOCK_PER_SECTOR; offset_index++)
	{
		if (block_read_buffer[offset_index] != NULL) //��� ������ �о���� ���ۿ��� �ش� ��ġ�� ������� ������, �� ��ȿ�� �������̸�
		{
			if (Flash_write(flashmem, PBN1_block_meta_buffer_array[offset_index], (empty_spare_block_num * BLOCK_PER_SECTOR) + offset_index, block_read_buffer[offset_index]) == COMPLETE)
				goto OVERWRITE_ERR;
		}
		else //��ȿ�ϰų�, ����ִ� ��ġ
		{
			//do nothing
		}
	}
	if (block_read_buffer[0] == NULL) //������ 0���� ����ִٸ� ��� ������ ������� �ʾ����Ƿ� ��� ������ ���� ����
		SPARE_write(flashmem, (empty_spare_block_num * BLOCK_PER_SECTOR), PBN1_block_meta_buffer_array[0]);

	if (deallocate_block_meta_buffer_array(PBN1_block_meta_buffer_array) != SUCCESS)
		goto MEM_LEAK_ERR;

	/*** Wear-leveling�� ���� ���� PBN1�� Merge���� ���� Spare Block�� ��ü : ***/
	//Normal Block : NORMAL_BLOCK_EMPTY => SPARE_BLOCK_EMPTY
	SPARE_read(flashmem, (PBN1 * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����
	meta_buffer->set_block_state(BLOCK_STATE::SPARE_BLOCK_EMPTY);
	SPARE_write(flashmem, (PBN1 * BLOCK_PER_SECTOR), meta_buffer);

	if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;

	SWAP(flashmem->log_block_level_mapping_table[LBN][0], flashmem->spare_block_queue->queue_array[spare_block_queue_index], tmp); //PBN1, Spare Block ��ü

	/*** Wear-leveling�� ���� ���� PBN2�� ����ִ� Spare Block�� ��ü ***/
	if (flashmem->spare_block_queue->dequeue(flashmem, empty_spare_block_num, spare_block_queue_index) == FAIL)
		goto SPARE_BLOCK_EXCEPTION_ERR;
	
	//Spare Block : SPARE_BLOCK_EMPTY => NORMAL_BLOCK_EMPTY
	SPARE_read(flashmem, (empty_spare_block_num * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����
	meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_EMPTY);
	SPARE_write(flashmem, (empty_spare_block_num* BLOCK_PER_SECTOR), meta_buffer);
	
	if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;

	//Normal Block : NORMAL_BLOCK_EMPTY => SPARE_BLOCK_EMPTY
	SPARE_read(flashmem, (PBN2 * BLOCK_PER_SECTOR), meta_buffer); //����� ù ��° ������ Spare ������ ����
	meta_buffer->set_block_state(BLOCK_STATE::SPARE_BLOCK_EMPTY);
	SPARE_write(flashmem, (PBN2 * BLOCK_PER_SECTOR), meta_buffer);

	if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
		goto MEM_LEAK_ERR;

	flashmem->spare_block_queue->queue_array[spare_block_queue_index] = PBN2;

	if (table_type == TABLE_TYPE::DYNAMIC)
		flashmem->empty_block_queue->enqueue(empty_spare_block_num); //Empty Block ��⿭�� ��ü�� Spare Block �߰� (Dynamic Table)

	/*** PBN2 ��� ���� ���̺�, ������ ���� ���̺� �ʱ�ȭ ***/
	for (__int8 offset_index = 0;  offset_index < BLOCK_PER_SECTOR;  offset_index++)
	{
		offset_level_table_index = (PBN2 * BLOCK_PER_SECTOR) + offset_index; //������ ���� ���̺� �������� index��
		flashmem->offset_level_mapping_table[offset_level_table_index] = OFFSET_MAPPING_INIT_VALUE;
	}
	flashmem->log_block_level_mapping_table[LBN][1] = DYNAMIC_MAPPING_INIT_VALUE;

	flashmem->save_table(mapping_method);

	return SUCCESS;

SPARE_BLOCK_EXCEPTION_ERR:
	if (VICTIM_BLOCK_QUEUE_RATIO != SPARE_BLOCK_RATIO)
		fprintf(stderr, "Spare Block Queue�� �Ҵ�� ũ���� ���� ��� ��� : �̱���, GC�� ���� ó���ǵ��� �ؾ��Ѵ�.\n");
	else
	{
		fprintf(stderr, "ġ���� ���� : Spare Block Queue �� GC Scheduler�� ���� ���� �߻� (FTL_write)\n");
		system("pause");
		exit(1);
	}
	return FAIL;

WRONG_ASSIGNED_LBN_ERR:
	fprintf(stderr, "ġ���� ���� : �߸� �Ҵ� �� LBN (full_merge)\n");
	system("pause");
	exit(1);

WRONG_META_ERR:
	fprintf(stderr, "ġ���� ���� : �߸��� meta ���� (full_merge)\n");
	system("pause");
	exit(1);

OVERWRITE_ERR:
	fprintf(stderr, "ġ���� ���� : Overwrite�� ���� ���� �߻� (full_merge)\n");
	system("pause");
	exit(1);

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (full_merge)\n");
	system("pause");
	exit(1);
}

int full_merge(FlashMem*& flashmem, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //���̺��� �����Ǵ� ��� ��Ͽ� ���� Merge ����
{
	switch (mapping_method) //���� ����
	{
	case MAPPING_METHOD::HYBRID_LOG:
		break;

	default:
		return FAIL;
	}
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	f_flash_info = flashmem->get_f_flash_info();

	for (unsigned int LBN = 0; LBN < (f_flash_info.block_size - f_flash_info.spare_block_size); LBN++)
	{
		//�Ϲ� ��� ���� ���� ���̺��� �����Ǵ� ��� �Ϲ� ��Ͽ� ���� ������ ��� Merge ����
		if (full_merge(flashmem, LBN, mapping_method, table_type) == FAIL)
			return FAIL;
	}

	return SUCCESS;
}

int trace(FlashMem*& flashmem, MAPPING_METHOD mapping_method, TABLE_TYPE table_type) //Ư�� ���Ͽ� ���� ���� ������ �����ϴ� �Լ�
{
	//��ü trace �� �����ϱ� ���� ����� read, write, erase Ƚ�� ���

	FILE* trace_file_input = NULL; //trace ���� �Է� ����
	
#if defined PAGE_TRACE_MODE || defined BLOCK_TRACE_MODE
	F_FLASH_INFO f_flash_info;
	f_flash_info = flashmem->get_f_flash_info();
#endif

	char file_name[FILENAME_MAX]; //trace ���� �̸�
	char command[2] = { 0, }; //��ɾ� (w,r,e) : '\0' ���� 2�ڸ�
	unsigned int LSN = UINT32_MAX; //LSN
	char dummy_data = 'A'; //trace�� ���� ���� ������

#ifdef DEBUG_MODE
	unsigned int count = 0; //����� ���� ���� ī��Ʈ
#endif

	system("cls");
	std::cout << "< ���� ���� ��� >" << std::endl;
	system("dir");
	std::cout << "trace ���� �̸� �Է� (�̸�.Ȯ����) >>";
	gets_s(file_name, FILENAME_MAX);

	trace_file_input = fopen(file_name, "rt"); //�б� + �ؽ�Ʈ ���� ���

	if (trace_file_input == NULL)
	{
		fprintf(stderr, "File does not exist or cannot be opened.\n");
		return FAIL;
	}

	switch (flashmem->v_flash_info.flash_state)
	{
	case FLASH_STATE::IDLE:
		flashmem->v_flash_info.flash_state = FLASH_STATE::WRITES; //���ӵ� ���� �۾� ���� ������ �˸�

	default:
		break;
	}

	std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now(); //trace �ð� ���� ����
	while (!feof(trace_file_input))
	{
		memset(command, NULL, sizeof(command));
		LSN = UINT32_MAX; //���� ������ �÷��� �޸��� �뷮���� ���� �� ���� ���� LSN ������ �ʱ�ȭ

		fscanf(trace_file_input, "%s\t%u\n", &command, &LSN); //������ �и��� ���� �б�
		
		if (strcmp(command, "w") == 0 || strcmp(command, "W") == 0) //Write LSN
		{
			FTL_write(flashmem, LSN, dummy_data, mapping_method, table_type);

#ifdef DEBUG_MODE
			count++;
			printf("Debug========================\n");
			printf("written_sector_count : %u\n", flashmem->v_flash_info.written_sector_count);
			printf("invalid_sector_count : %u\n", flashmem->v_flash_info.invalid_sector_count);
			printf("current count : %u\n", count);
			printf("=============================\n");

			//if (count == 5)
			//	system("pause");
#endif
		}

	}
	fclose(trace_file_input);
	std::chrono::system_clock::time_point end_time = std::chrono::system_clock::now(); //trace �ð� ���� ��
	std::chrono::milliseconds mill_end_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << ">> Trace function ends : " << mill_end_time.count() <<"milliseconds elapsed"<< std::endl;

#ifdef PAGE_TRACE_MODE //Trace for Per Sector(Page) Wear-leveling
	FILE* trace_per_page_output = NULL; //����(������) �� trace ��� ���
	trace_per_page_output = fopen("trace_per_page_result.txt", "wt");

	if (trace_per_page_output == NULL)
	{
		fprintf(stderr, "����(������) �� ���� ��� ���� ���� (trace_per_page_result.txt)\n");
		return FAIL;
	}

	fprintf(trace_per_page_output, "PSN\tRead\tWrite\tErase\n");
	for (unsigned int PSN = 0; PSN < f_flash_info.sector_size; PSN++)
	{
		//PSN [TAB] �б� Ƚ�� [TAB] ���� Ƚ�� [TAB] ����� Ƚ�� ��� �� ���� trace�� ���� �ʱ�ȭ ���� ����
		fprintf(trace_per_page_output, "%u\t%u\t%u\t%u\n", PSN, flashmem->page_trace_info[PSN].read_count,
			flashmem->page_trace_info[PSN].write_count, flashmem->page_trace_info[PSN].erase_count);

		flashmem->page_trace_info[PSN].clear_all(); //�б�, ����, ����� Ƚ�� �ʱ�ȭ
	}
	fclose(trace_per_page_output);

	printf(">> ����(������) �� ���� ���� trace_per_page_result.txt\n");
	system("notepad trace_per_page_result.txt");
#endif

#ifdef BLOCK_TRACE_MODE //Trace for Per Block Wear-leveling
	FILE* trace_per_block_output = NULL; //��� �� trace ��� ���
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
		fprintf(trace_per_block_output, "%u\t%u\t%u\t%u\n", PBN, flashmem->block_trace_info[PBN].read_count,
		flashmem->block_trace_info[PBN].write_count, flashmem->block_trace_info[PBN].erase_count);
		
		flashmem->block_trace_info[PBN].clear_all(); //�б�, ����, ����� Ƚ�� �ʱ�ȭ
	}
	fclose(trace_per_block_output);

	printf(">> ��� �� ���� ���� trace_per_block_result.txt\n");
	system("notepad trace_per_block_result.txt");
#endif

	switch (flashmem->v_flash_info.flash_state)
	{
	case FLASH_STATE::WRITES:
		flashmem->v_flash_info.flash_state = FLASH_STATE::IDLE; //���� �������� �˸�

	default:
		break;
	}

	return SUCCESS;
}