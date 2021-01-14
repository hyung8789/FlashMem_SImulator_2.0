#include "Build_Options.h"

// Garbage Collecter ������ ���� Ŭ���� ����

GarbageCollector::GarbageCollector() 
{
	this->RDY_terminate = false;
}

GarbageCollector::~GarbageCollector() {}

int GarbageCollector::scheduler(class FlashMem*& flashmem, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type) //main scheduling function for GC
{
	F_FLASH_INFO f_flash_info;

	if (flashmem == NULL || mapping_method == MAPPING_METHOD::NONE) //�÷��� �޸𸮰� �Ҵ�Ǿ��ְ�, ���� ����� ����� ��쿡�� ����
		goto END_EXE_COND_EXCEPTION;

	switch (this->RDY_terminate)
	{
	case true: //�ý��� ���� ��� ����
		goto TERMINATE_PROC;

	case false:
		break;
	}

	switch (flashmem->v_flash_info.flash_state)
	{
	case FLASH_STATE::INIT: //������ �÷��� �޸� ������ �籸�� �������� ���� ��ȿȭ�� ��Ͽ� ���ؼ� Victim Block ��⿭�� ���Ը� ����, �̿� ����, ��⿭�� ���� �� ��� ��ü ó��
		this->enqueue_job(flashmem, mapping_method, table_type);

		if (flashmem->victim_block_queue->is_full())
			this->all_dequeue_job(flashmem, mapping_method, table_type); //VIctim Block ť ���� ��� Victim Block�鿡 ���� ó��

		return SUCCESS;
	
	default:
		break;
	}

	f_flash_info = flashmem->get_f_flash_info(); //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	printf("\n-----------------------------------\n");
	printf("�� Starting GC Scheduling Task...\n");

	if (this->enqueue_job(flashmem, mapping_method, table_type) == SUCCESS)
		printf("- enqueue job performed\n");

	switch (flashmem->v_flash_info.flash_state) //���� �÷��� �޸��� �۾� ���¿� ����
	{
	case FLASH_STATE::IDLE: //When the write operation ends
		if (flashmem->victim_block_queue->is_empty()) //Victim Block ��⿭�� �� ���
		{
			if(mapping_method == MAPPING_METHOD::HYBRID_LOG)
				if (flashmem->empty_block_queue->is_empty()) //��� �� �� �ִ� �� ���� ����� ������
				{
					full_merge(flashmem, mapping_method, table_type); //���̺� �����Ͽ� ��� ��Ͽ� ���Ͽ� ���� �� ��� Merge ����
					goto END_SUCCESS;
				}

			goto END_COMPLETE; //do nothing
		}
		else //������� �ʰų�, ���� �� ���
		{
			this->all_dequeue_job(flashmem, mapping_method, table_type); //Victim Block ��⿭ ���� ��� Victim Block�鿡 ���� ó��
			printf("- all dequeue job performed\n");

			goto END_SUCCESS;
		}
	case FLASH_STATE::WRITES: //When write operation occurs
	case FLASH_STATE::WRITE: //When write operation occurs
		if (flashmem->victim_block_queue->is_full()) //Victim Block ��⿭�� ���� �� ���
		{
			this->all_dequeue_job(flashmem, mapping_method, table_type); //Victim Block ��⿭ ���� ��� Victim Block�鿡 ���� ó��
			printf("- all dequeue job performed\n");

			goto END_SUCCESS;
		}

		goto END_COMPLETE; //do nothing

	case FLASH_STATE::FORCE_GC: //���� ������ �÷��� �ǽ�
		if (mapping_method == MAPPING_METHOD::HYBRID_LOG)
			full_merge(flashmem, mapping_method, table_type);

		this->all_dequeue_job(flashmem, mapping_method, table_type); //Victim Block ��⿭ ���� ��� Victim Block�鿡 ���� ó��
		printf("- all dequeue job performed (Force GC)\n");

		goto END_SUCCESS;
	}

END_EXE_COND_EXCEPTION:
	return FAIL;

END_COMPLETE:
	printf("End with no Operation\n");
	printf("-----------------------------------\n");
	return COMPLETE;

END_SUCCESS:
	printf("Success\n");
	printf("-----------------------------------\n");
	return SUCCESS;

NO_PHYSICAL_SPACE_EXCEPTION_ERR:
	printf("All physical spaces are used\n");
	printf("-----------------------------------\n");
	system("pause");
	exit(1);

TERMINATE_PROC:
	this->all_dequeue_job(flashmem, mapping_method, table_type); //��� Victim Block�� ���ͼ� ó��
	flashmem->save_table(mapping_method);
	delete flashmem;

	exit(1);
}


int GarbageCollector::one_dequeue_job(class FlashMem*& flashmem, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type) //Victim Block ť�κ��� �ϳ��� Victim Block�� ���ͼ� ó��
{
	//��� ���� : �ش� Victim Block�� �׻� ��ȿȭ�Ǿ� �����Ƿ� �ܼ� Erase ����
	//���̺긮�� ���� : ��ȿȭ�� ����� ��� �ܼ� Erase, �ƴ� ��� Merge ����

	victim_block_element victim_block;
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	spare_block_num empty_spare_block_num = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int empty_spare_block_index = DYNAMIC_MAPPING_INIT_VALUE;

	/***
		<��� ����>

		Overwrite�߻� �� �ش� PBN�� ������ ��ȿȭ, Wear-leveling�� ���� Spare Block�� ��ü�Ͽ� ���ο� ������ �� ���� ��ȿ �����͵�
		��� �� ���� ����� Victim Block���� ���� => �ش� PBN�� ���� Erase ����
		
		<���̺긮�� ����>

		1) Log Algorithm�� ������ ���̺긮�� ������ LBN�� ������ PBN1 �Ǵ� PBN2�� ��� �����Ͱ� ��ȿȭ�ɽÿ� ��� ���� ���� ���̺��� ���� �� Victim Blok���� ����
		2) Wear-leveling�� ���� Spare Block�� ��ü, Victim Block�� Erase ���� �� Spare Block ��⿭�� �߰�, ��ü �� Spare Block�� Empty Block ��⿭�� �߰�

		LBN�� PBN1, PBN2 ��� �����Ǿ� �ְ�(��, �����̶� ����� ��ȿȭ��������), ��ϰ��� Ȯ���� ���� LBN�� Victim Block���� ���� => �ش� LBN�� ���� Merge ����
		
		����, ��� ������ ������ ��ȿ �����͵�� ��� �Ǿ� �ְ� (Empty Block Queue�� �� ����� �������� ����),
		�� �̻� ���ο� �� ���� ����� �Ҵ� �� �� ���� ��Ȳ����, ���� �����Ϳ� ���� Overwrite�� �߻��Ѵٸ�,
		��� ������ ���ó��, Spare Block�� ��ü�Ͽ� ���ο� ������ �� ���� ��ȿ �����͵� ��� �� ���� ����� Victim Block���� ���� => �ش� PBN�� ���� Erase ����
		---
		=> ��� ������ ��� Overwrite �� ������ �� Spare Block�� Ȱ���Ͽ� ��� ���� �� ���� ����� ��ȿȭ ó���Ǿ� Wear-leveling�� ���� �ٸ� �� Spare Block�� ��ü�� �����Ѵ�.
		�̿� ���Ͽ�, ���̺긮�� ������ ��� Overwrite �� PBN1 �Ǵ� PBN2�� ��� �����Ͱ� ��ȿȭ�� �ÿ� PBN1�� ��� PBN2�� ���ο� �����Ͱ� ��� �� ���̰�, PBN2�� ��� PBN1�� ��ϵ� ���̴�.
		��ȿȭ ó���� ����� Spare Block�� ��ü �۾��� ���� �ϰ�, 
		Victim Block���� �����ϵ�, ��� ���� Ȯ���� ���� (�̸� ��ü�Ͽ� �� ����� ���� ��ų ���, �ٸ� LBN���� ��� �Ұ���)
		�ٷ� ������ Spare Block�� ��ü�� �������� �ʰ� ���� ���̺��� Unlink�� �����Ѵ�.
		GC�� ���� ��ȿȭ ó���� ����� Erase�ϰ�, Wear-leveling�� ���� ������ �� Spare Block�� ��ü�Ѵ�.
	***/
	if (flashmem->victim_block_queue->dequeue(victim_block) == SUCCESS)
	{
		switch (victim_block.proc_state) //�ش� Victim Block�� ���� ó���� ���¿� ����
		{
		case VICTIM_BLOCK_PROC_STATE::UNLINKED:
			/***
				�ش� ���(PBN)�� ��� ���� ���̺��� �����Ǿ� ���� ����
				erase ���� �� 0xff ������ ��� �ʱ�ȭ�ǹǷ�,
				erase ���� ��(NORMAL_BLOCK_INVALID) => erase ���� ��(NORMAL_BLOCK_EMPTY) => SPARE_BLOCK_EMPTY�� ���� �� Wear-leveling�� ���� Spare Block�� ��ü, �ش� ����� �� ��� ��⿭�� �߰�
			***/
			Flash_erase(flashmem, victim_block.victim_block_num);

			SPARE_read(flashmem, (victim_block.victim_block_num * BLOCK_PER_SECTOR), meta_buffer);
			meta_buffer->set_block_state(BLOCK_STATE::SPARE_BLOCK_EMPTY);
			SPARE_write(flashmem, (victim_block.victim_block_num * BLOCK_PER_SECTOR), meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			/*** �ش� ����� ��� ���� ���̺��� �����Ǿ� ���� �ʴ�. Wear-leveling�� ���Ͽ� Victim Block�� Spare Block�� ��ü ***/
			if (flashmem->spare_block_queue->dequeue(flashmem, empty_spare_block_num, empty_spare_block_index) == FAIL)
				goto SPARE_BLOCK_EXCEPTION_ERR;

			flashmem->spare_block_queue->queue_array[empty_spare_block_index] = victim_block.victim_block_num;

			SPARE_read(flashmem, (empty_spare_block_num * BLOCK_PER_SECTOR), meta_buffer);
			meta_buffer->set_block_state(BLOCK_STATE::NORMAL_BLOCK_EMPTY);
			SPARE_write(flashmem, (empty_spare_block_num * BLOCK_PER_SECTOR), meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			if (table_type == TABLE_TYPE::DYNAMIC) //Victim Block�� ��𿡵� �������� �ʴ� ���� Dynamic Table ���ۿ� ����
				flashmem->empty_block_queue->enqueue(empty_spare_block_num); //��ü �� Spare Block�� Empty Block ��⿭�� �߰� (Dynamic Table)

			break;

		case VICTIM_BLOCK_PROC_STATE::SPARE_LINKED:
			/***
				�ش� ����� ����ִ� Spare Block�� ����Ͽ� ������ ��ȿ ������ �� ���ο� ������ ����� �����ϱ� ���� ��ü �۾��� ����Ǿ�, Spare Block ��⿭�� �����Ǿ� ����
				erase ���� �� 0xff ������ ��� �ʱ�ȭ�ǹǷ�,
				erase ���� ��(NORMAL_BLOCK_INVALID) => erase ���� ��(NORMAL_BLOCK_EMPTY) => SPARE_BLOCK_EMPTY�� ����
			***/

			Flash_erase(flashmem, victim_block.victim_block_num);

			SPARE_read(flashmem, (victim_block.victim_block_num * BLOCK_PER_SECTOR), meta_buffer);
			meta_buffer->set_block_state(BLOCK_STATE::SPARE_BLOCK_EMPTY);
			SPARE_write(flashmem, (victim_block.victim_block_num * BLOCK_PER_SECTOR), meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 

			if (deallocate_single_meta_buffer(meta_buffer) != SUCCESS)
				goto MEM_LEAK_ERR;

			break;

		case VICTIM_BLOCK_PROC_STATE::UNPROCESSED_FOR_MERGE: //���� ������� �ʴ� ���
			if (!victim_block.is_logical) //���� ��� ��ȣ(PBN)�� ��� ����
				goto WRONG_VICTIM_BLOCK_PROC_STATE;

			full_merge(flashmem, victim_block.victim_block_num, mapping_method, table_type);

			break;

		default:
			goto WRONG_VICTIM_BLOCK_PROC_STATE;
		}
	}
	else
		return FAIL;

END_SUCCESS:
	return SUCCESS;

SPARE_BLOCK_EXCEPTION_ERR:
	if (VICTIM_BLOCK_QUEUE_RATIO != SPARE_BLOCK_RATIO)
		fprintf(stderr, "Spare Block Queue�� �Ҵ�� ũ���� ���� ��� ��� : �̱���, GC�� ���� ó���ǵ��� �ؾ��Ѵ�.\n");
	else
	{
		fprintf(stderr, "ġ���� ���� : Spare Block Queue �� GC Scheduler�� ���� ���� �߻� (one_dequeue_job)\n");
		system("pause");
		exit(1);
	}
	return FAIL;

MEM_LEAK_ERR:
	fprintf(stderr, "ġ���� ���� : meta ������ ���� �޸� ���� �߻� (one_dequeue_job)\n");
	system("pause");
	exit(1);

WRONG_VICTIM_BLOCK_PROC_STATE:
	fprintf(stderr, "ġ���� ���� : Wrong VICTIM_BLOCK_PROC_STATE (one_dequeue_job)\n");
	system("pause");
	exit(1);
}

int GarbageCollector::all_dequeue_job(class FlashMem*& flashmem, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type) //Victim Block ť�� ��� Victim Block�� ���ͼ� ó��
{
	while (1) //ť�� �� ������ �۾�
	{
		if((this->one_dequeue_job(flashmem, mapping_method, table_type) != SUCCESS))
			break;
	}

	return SUCCESS;
}


int GarbageCollector::enqueue_job(class FlashMem*& flashmem, enum MAPPING_METHOD mapping_method, enum TABLE_TYPE table_type) //Victim Block ť�� ����
{
	if (flashmem->victim_block_info.victim_block_num != DYNAMIC_MAPPING_INIT_VALUE)
	{
		(flashmem->victim_block_queue->enqueue(flashmem->victim_block_info));

		//��� �� ���� Victim Block ���� ���� ���� �ʱ�ȭ
		flashmem->victim_block_info.clear_all();
		return SUCCESS;
	}

	return FAIL;
}