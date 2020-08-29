#include "GarbageCollector.h"

// Garbage Collecter ������ ���� Ŭ���� ����

GarbageCollector::GarbageCollector() 
{
	this->invalid_ratio_threshold = 1.0;
	this->RDY_v_flash_info_for_set_invalid_ratio_threshold = false; //������ �÷��� �޸� ���� ���� �Ϸ� �� true�� set
	this->gc_lazy_mode = true; //��� ���� ���� �� false�� set
	this->RDY_terminate = false;
}

GarbageCollector::~GarbageCollector() {}

void GarbageCollector::print_invalid_ratio_threshold()
{
	printf("Current Invalid Ratio Threshold : %f\n", this->invalid_ratio_threshold);
}

int GarbageCollector::scheduler(FlashMem** flashmem, int mapping_method) //main scheduling function for GC
{
	unsigned int written_sector_count = 0;
	F_FLASH_INFO f_flash_info;
	unsigned int physical_using_space = 0;
	unsigned int physical_free_space = 0;
	unsigned int logical_using_space = 0;
	unsigned int logical_free_space = 0;
	bool flag_vq_is_full = false;
	bool flag_vq_is_empty = false;

	if ((*flashmem) == NULL || mapping_method == 0) //�÷��� �޸𸮰� �Ҵ�Ǿ��ְ�, ���� ����� ����� ��쿡�� ����
		return FAIL;

	switch (this->RDY_terminate)
	{
	case true: //���� ��� ����
		goto TERMINATE_PROC;
		break;

	case false:
		break;
	}

	switch (this->RDY_v_flash_info_for_set_invalid_ratio_threshold)
	{
	case true:
		break;

	case false:  //Reorganization�� ���� ������ �÷��� �޸� ������ �غ���� �ʾ�����
		//���� ��ȿȭ�� ��Ͽ� ���ؼ� Victim Block ť�� ���Ը� ����, �̿� ����, ť�� ���� �� ��� ��ü ó��
		this->enqueue_job(flashmem, mapping_method);

		if ((*flashmem)->victim_block_queue->is_full() == true)
			this->all_dequeue_job(flashmem, mapping_method); //VIctim Block ť ���� ��� Victim Block�鿡 ���� ó��

		return SUCCESS;
		break;
	}

	printf("\n�� Starting GC Scheduling Task...");
	/***
		���� �÷��� �޸��� ���� ���� ������ ���� Victim Block ���� ���� ��ȿ�� �Ӱ谪 ���
		Victim Block ���� ���� ���� ���� �� ��ȿ�� �Ӱ谪�� ���� Victim Block ť�� ����
	***/
	this->set_invalid_ratio_threshold(flashmem);
	this->enqueue_job(flashmem, mapping_method);

	/***
		���� �÷��� �޸��� ��ȿ ������ ���� �� ��� ���� ���� Ȯ��
		���� Victim Block ť Ȯ��(is_full, is_empty)
	***/
	written_sector_count = (*flashmem)->v_flash_info.written_sector_count; //���� �÷��� �޸��� ��ϵ� ���� ��
	f_flash_info = (*flashmem)->get_f_flash_info(); //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	/***
		���������� �����ִ� ��� ���� ���� = ��ü byte���� �� - (��ϵ� ���� �� * ���� �� ����Ʈ ��)
		=> ����ڿ��� �������� �ʴ� �뷮�̹Ƿ�, Spare Block�� ���Խ�Ų��.

		�������� �����ִ� ��� ������ ���� ������ ������ ����� �Ұ����� ������ Spare Block�� �����ϴ� �� byte���� �����Ѵ�
	***/

	physical_using_space = (*flashmem)->v_flash_info.written_sector_count * SECTOR_INC_SPARE_BYTE; //���������� ��� ���� ����
	physical_free_space = f_flash_info.storage_byte - physical_using_space; //���������� �����ִ� ��� ���� ����

	//�������� �����ִ� ��� ���� ���� = ��ü byte���� �� - (��ϵ� ���͵� �� ��ȿ ���� ���� * ���� �� ����Ʈ ��) - Spare Block�� �����ϴ� �� byte��
	logical_using_space = ((*flashmem)->v_flash_info.written_sector_count - (*flashmem)->v_flash_info.invalid_sector_count) * SECTOR_INC_SPARE_BYTE;
	logical_free_space = (f_flash_info.storage_byte - logical_using_space) - f_flash_info.spare_block_byte;

	flag_vq_is_full = (*flashmem)->victim_block_queue->is_full();
	flag_vq_is_empty = (*flashmem)->victim_block_queue->is_empty();

	//���� ���������� �����ִ� ��� ���� ������ ���� ���
	if(physical_free_space == 0)
	{
		//��� ���� ���� �� ��� ���� Ȯ���� ���� Lazy Mode ��Ȱ��ȭ
		switch (this->gc_lazy_mode)
		{
		case true:
			this->gc_lazy_mode = false;
			break;

		case false:
			break;
		}

		if (flag_vq_is_full == true || flag_vq_is_empty == false) //Victim Block ť�� ���� �� �ְų� ������� ���� ���
		{
			this->all_dequeue_job(flashmem, mapping_method); //VIctim Block ť ���� ��� Victim Block�鿡 ���� ó��
			printf("all dequeue job performed\n");
		}
		else if (flag_vq_is_empty == true && mapping_method == 3) //Victim Block ť�� ����ְ�, ���̺긮�� ������ ���
		{
			full_merge(flashmem, mapping_method); //���̺� ���� ��ü ��Ͽ� ���� ���� �� ��� Merge ���� (Log Algorithm�� ������ ���̺긮�� ������ ��쿡�� ����)
			printf("full merge performed to all blocks\n");
		}
		else
		{
			/***
				��� ������ ��� Merge �Ұ���, Erase�� ����
				Overwrite �߻� �� �ش� ����� �׻� ��ȿȭ�ǹǷ�, ����(������)������ ��ȿȭ�� �����Ͱ� �������� �ʴ�.
				����, �� ��� ����ڿ��� �������� �뷮�� ���� ��������� ��� ����Ͽ��� �� �̻� ����� �Ұ����� ����̴�.
			***/
			printf("End with no Operation\n(All logical spaces are used)\n");
			return FAIL;
		}
	}
	else //���� ���������� �����ִ� ��� ���� ������ ������ ���� ���
	{
		//Lazy Mode Ȱ��ȭ
		switch (this->gc_lazy_mode)
		{
		case true:
			break;

		case false:
			this->gc_lazy_mode = true;
			break;
		}

		if (flag_vq_is_empty == true) //Victim Block ť�� �� ���
		{
			//�ƹ��� �۾��� ���� ����
			printf("End with no Operation (Lazy mode)\n");
			return COMPLETE;
		}
		else if (flag_vq_is_full == false && flag_vq_is_empty == false) //Victim Block ť�� ���� �� ���� �ʰ�, ������� ���� ���
		{
			switch (this->gc_lazy_mode)
			{
			case true:
				//���ӵ� ���� �۾��� ���� Write Performance ����� ���Ͽ� Victim Block ť�� ���� �� ������ �ƹ��� �۾��� �������� �ʴ´�.
				printf("End with no Operation (Lazy mode)\n");
				return COMPLETE;

			case false:
				this->one_dequeue_job(flashmem, mapping_method); //�ϳ��� �� �ͼ� ó��
				printf("one dequeue job performed (Lazy mode)\n");
				break;
			}
		}
		else //Victim Block ť�� ���� �� ���
		{
			this->all_dequeue_job(flashmem, mapping_method); //��� Victim Block�� ���ͼ� ó��
			printf("all dequeue job performed (Lazy mode)\n");
		}
	}

	printf("Success\n");
	return SUCCESS;

TERMINATE_PROC:
	this->all_dequeue_job(flashmem, mapping_method); //��� Victim Block�� ���ͼ� ó��
	(*flashmem)->save_table(mapping_method);
	exit(1);
}


int GarbageCollector::one_dequeue_job(class FlashMem** flashmem, int mapping_method) //Victim Block ť�κ��� �ϳ��� Victim Block�� ���ͼ� ó��
{
	//��� ���� : �ش� Victim Block�� �׻� ��ȿȭ�Ǿ� �����Ƿ� �ܼ� Erase ����
	//���̺긮�� ���� : ��ȿȭ�� ����� ��� �ܼ� Erase, �ƴ� ��� Merge ����

	victim_element victim_block; //ť���� ��Ҹ� ���ͼ� ����
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����
	META_DATA* meta_buffer = NULL; //Spare area�� ��ϵ� meta-data�� ���� �о���� ����

	spare_block_element empty_spare_block_for_SWAP = DYNAMIC_MAPPING_INIT_VALUE;
	unsigned int empty_spare_block_index = DYNAMIC_MAPPING_INIT_VALUE;

	/***
		��� ������ Overwrite�߻� �� �ش� PBN�� ������ ��ȿȭ, Wear-leveling�� ���� Spare Block�� ��ü �� Victim Block���� ���� => �ش� PBN�� ���� Erase����	
		Log Algorithm�� ������ ���̺긮�� ������ LBN�� ������ PBN1 �Ǵ� PBN2�� ��� �����Ͱ� ��ȿȭ�ɽÿ� Victim Block���� ���� => �ش� PBN�� ���� Erase ���� �� Wear-leveling�� ���� Spare Block�� ��ü
		LBN�� PBN1, PBN2 ��� �����Ǿ� �ְ�(��, �����̶� ����� ��ȿȭ��������), ��ϰ��� Ȯ���� ���� LBN�� Victim Block���� ���� => �ش� LBN�� ���� Merge ����
		---
		=> ��� ������ ��� Overwrite �� ������ �� Spare Block�� Ȱ���Ͽ� ��� ���� �� ���� ����� ��ȿȭ ó���Ǿ� Wear-leveling�� ���� �ٸ� �� Spare Block�� ��ü�� �����Ѵ�.
		�̿� ���Ͽ�, ���̺긮�� ������ ��� Overwrite �� PBN1 �Ǵ� PBN2�� ��� �����Ͱ� ��ȿȭ�� �ÿ� PBN1�� ��� PBN2�� ���ο� �����Ͱ� ��� �� ���̰�, PBN2�� ��� PBN1�� ��ϵ� ���̴�.
		��ȿȭ ó���� ����� Victim Block���� �����ϵ�, ��� ���� Ȯ���� ���� �ٷ� ������ Spare Block�� ��ü�� �������� �ʰ� ���� ���̺��� Unlink�� �����Ѵ�. 
		GC�� ���� ��ȿȭ ó���� ����� Erase�ϰ�, Wear-leveling�� ���� ������ �� Spare Block�� ��ü�Ѵ�.
	***/
	if ((*flashmem)->victim_block_queue->dequeue(victim_block) == SUCCESS)
	{
		switch (mapping_method)
		{
		case 2: //��� ����
			if (victim_block.victim_block_invalid_ratio != 1.0 || victim_block.is_logical == true) //Overwrite �߻� �� �׻� �ش� ����� ���� ��ȿȭ�ǹǷ� ��ȿ���� 1.0�� �ƴϸ� ����
				goto WRONG_INVALID_RATIO_ERR;
			
			Flash_erase(flashmem, victim_block.victim_block_num);
			/*** Spare Block���� ���� ***/
			meta_buffer = SPARE_read(flashmem, (victim_block.victim_block_num * BLOCK_PER_SECTOR));
			meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = false;
			SPARE_write(flashmem, (victim_block.victim_block_num * BLOCK_PER_SECTOR), &meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 
			delete meta_buffer;
			meta_buffer = NULL;

			break;

		case 3: //���̺긮�� ����(Log algorithm - 1:2 Block level mapping with Dynamic Table)
			/***
					Victim Block���� ������ ����� ���� ����� ���, �ش� ���� ���(PBN)�� ���� ��ȿȭ�Ǿ��⿡ Victim Block���� �����Ǿ���.
					�̿� ���Ͽ�, ��ȿ�� �Ӱ谪�� ���� ������ �� ���(LBN)�� �Ϻ� ��ȿ �� ��ȿ �����͸� �����ϰ� �ְ�, �ش�
					LBN�� ������ PBN1, PBN2�� ���Ͽ� Merge�Ǿ�� �Ѵ�.
					---
					����, Log Algorithm�� ������ ���̺긮�� ���ο��� �Ϻ� ��ȿ �� ��ȿ �����͸� �����ϰ� �ִ� ���� ���� ���(PBN1 �Ǵ� PBN2)�� ���ؼ���
					��� ���� Ȯ���� ���Ͽ�, ��ȿ�� �Ӱ谪�� ���� ���� �� ������ �� Spare ����� ����Ͽ� ��ȿ ������ copy �� Erase �� ��� ��ü �۾���
					�����Ѵٸ�, �ش� LBN�� PBN1�� PBN2�� ���� Merge�� �����ϴ� �Ͱ� ���Ͽ� �� ���� ��� ������ Ȯ���Ͽ�����, ��ȿ ������ copy �� 
					�ش� ���� ��� Erase�� ���� �� ���� ���߷� ���� ��ȿ�����̴�.
			***/

			if (victim_block.is_logical == true) //Victim Block ��ȣ�� LBN�� ��� : Merge ����
				full_merge(flashmem, victim_block.victim_block_num, mapping_method);
			else //Victim Block ��ȣ�� PBN�� ��� : Erase ����
			{		
				if (victim_block.victim_block_invalid_ratio != 1.0 || victim_block.is_logical == true)
					goto WRONG_INVALID_RATIO_ERR;

				Flash_erase(flashmem, victim_block.victim_block_num);
				/*** Spare Block���� ���� �� Wear-leveling�� ���� ��� ��ü ***/
				meta_buffer = SPARE_read(flashmem, (victim_block.victim_block_num * BLOCK_PER_SECTOR));
				meta_buffer->meta_data_array[(__int8)META_DATA_BIT_POS::not_spare_block] = false;
				SPARE_write(flashmem, (victim_block.victim_block_num * BLOCK_PER_SECTOR), &meta_buffer); //�ش� ����� ù ��° �������� meta���� ��� 
				delete meta_buffer;
				meta_buffer = NULL;

				/*** Wear-leveling�� ���Ͽ� �� Spare Block�� ��ü ***/
				if ((*flashmem)->spare_block_table->rr_read(flashmem, empty_spare_block_for_SWAP, empty_spare_block_index) == FAIL)
					goto SPARE_BLOCK_EXCEPTION_ERR;
				
				(*flashmem)->spare_block_table->table_array[empty_spare_block_index] = victim_block.victim_block_num; //���� ���̺� �������� ���� ����̹Ƿ� �ܼ� �Ҵ縸 ����
			}
			break;
		}
	}
	else
		return FAIL;

	return SUCCESS;

WRONG_INVALID_RATIO_ERR:
	fprintf(stderr, "���� : Wrong Invalid Ratio (%f)\n", victim_block.victim_block_invalid_ratio);
	system("pause");
	exit(1);

SPARE_BLOCK_EXCEPTION_ERR:
	if (VICTIM_BLOCK_QUEUE_RATIO != SPARE_BLOCK_RATIO)
		fprintf(stderr, "Spare Block Table�� �Ҵ�� ũ���� ���� ��� ��� : �̱���, GC�� ���� ó���ǵ��� �ؾ��Ѵ�.\n");
	else
	{
		fprintf(stderr, "���� : Spare Block Table �� GC Scheduler�� ���� ���� �߻� (FTL_write)\n");
		system("pause");
		exit(1);
	}
	return FAIL;
}

int GarbageCollector::all_dequeue_job(class FlashMem** flashmem, int mapping_method) //Victim Block ť�� ��� Victim Block�� ���ͼ� ó��
{
	while (1) //ť�� �� ������ �۾�
	{
		if((this->one_dequeue_job(flashmem, mapping_method) != SUCCESS))
			break;
	}

	return SUCCESS;
}


int GarbageCollector::enqueue_job(class FlashMem** flashmem, int mapping_method) //Victim Block ť�� ����
{
	/***
		Victim Block ���� ����ü �ʱⰪ
		---
		victim_block_num = DYNAMIC_MAPPING_INIT_VALUE;
		victim_block_invalid_ratio = -1;
	***/
	
	//Victim Block ���� ����ü�� �ʱⰪ�� �ƴϸ�, ��û�� �������Ƿ� �Ӱ谪�� ���� ó��
	if (((*flashmem)->victim_block_info.victim_block_num != DYNAMIC_MAPPING_INIT_VALUE) && (*flashmem)->victim_block_info.victim_block_invalid_ratio != -1)
	{
		if((*flashmem)->victim_block_info.victim_block_invalid_ratio >= this->invalid_ratio_threshold) //�Ӱ谪���� ���ų� ũ�� ����
			((*flashmem)->victim_block_queue->enqueue((*flashmem)->victim_block_info));

		//��� �� ���� Victim Block ���� ���� ���� �ʱ�ȭ
		(*flashmem)->victim_block_info.clear_all();
		return SUCCESS;
	}
	else
		return FAIL;
}

void GarbageCollector::set_invalid_ratio_threshold(class FlashMem** flashmem) //���� ��� ������ ���丮�� �뷮�� ���� ������ ��ȿ�� �Ӱ谪 ����
{
	F_FLASH_INFO f_flash_info = (*flashmem)->get_f_flash_info(); //�÷��� �޸� ���� �� �����Ǵ� ������ ����

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

	/***
		�����ǰ� ���� ó���� ���� ���� Victim Block ������ ������ ����, 
		���������� ������� ���� (Spare Block �� ��ȿ ������ ����) > �������� ������� ���� (Spare Block �� ��ȿ ������ ����)
		
		������ ��� Victim Block�鿡 ���� ó���� �ȴٸ�, 
		���������� ������� ���� (Spare Block �� ��ȿ ������ ����)�� �������� ������� ���� (Spare Block �� ��ȿ ������ ����)�� 
		��ġ������ ������ ���� ��������.
		
		ex) ���� :
			��ü ���丮�� ũ�� 100MB
			Spare Block���� �Ҵ�� ũ�� 10MB
			���������� �����ִ� ��� ���� : 100MB
			�������� �����ִ� ��� ���� : 90MB
			
			=> ��ϵ� ������ �뷮�� 50MB�̰�, ��ȿ �����Ͱ� �߰������� 20MB ����, Spare Block�� ��� ���� �ƴ϶��,
			
			���������� �����ִ� ��� ���� : 100MB - 50MB - 20MB = 30MB
			�������� �����ִ� ��� ���� : 90MB - 50MB = 40MB
			���������� ������� ��� ���� : 50MB + 20MB = 70MB
			�������� ������� ��� ���� :	50MB

			=> ��ϵ� ������ �뷮�� 90MB�̰�, ��ȿ �����Ͱ� ��ϵ� ������ �뷮�� ���� 50MB, Spare Block�� ��� ���� �ƴ϶��,

			���������� �����ִ� ��� ���� : 100MB - 90MB = 10MB(Spare Block���� �Ҵ�� �뷮)
			�������� �����ִ� ��� ���� : 90MB - 40MB(��ȿ ������) = 50MB
			���������� ��� ���� ��� ���� : 90MB(��ȿ ������ 40MB, ��ȿ ������ 50MB)
			�������� ��� ���� ��� ���� : 40MB
		---
		=> ���������� �����ִ� ��� ������ ���� ��ȿ�� �Ӱ谪�� �����Ѵٸ�, ������ ��� ������ Spare Block�� �����ϹǷ�,
		��ȿ �����Ͱ� Spare Block�� �����Ѵٸ�, ���������� �����ִ� ��� ������ Spare Block���� �Ҵ�� �뷮�� ������ �� ����.
		�̿� ����, �������� �����ִ� ��� ������ ���� ��ȿ�� �Ӱ谪 ���� ����
	***/

	/***
		- �ּ� �Ӱ谪 0.03125 (��� �ϳ��� ���� �� ������ �Ű澲�� �ʰ� �ϳ��� �������� ��ȿȭ�Ǿ��� ���� ��ȿ��)
		- �ִ� �Ӱ谪 1.0 (��� �ϳ��� ���� ��� �� ������ ��(32)��ŭ ��ȿȭ�Ǿ��� ���� ��ȿ��)
	***/

	try
	{
		float result_invalid_ratio_threshold = (float)logical_free_space / ((float)f_flash_info.storage_byte - (float)f_flash_info.spare_block_byte);

		if (result_invalid_ratio_threshold == 0)
			this->invalid_ratio_threshold = 0.03125; //�ּ� �Ӱ谪 ���� (1������ ��ȿȭ�� ��ȿ��)
		else if (result_invalid_ratio_threshold > 0 && result_invalid_ratio_threshold <= 1)
			this->invalid_ratio_threshold = result_invalid_ratio_threshold;
		else //�߸��� �Ӱ谪
			throw result_invalid_ratio_threshold;

		return;
	}
	catch (float result_invalid_ratio_threshold)
	{
		fprintf(stderr, "���� : �߸��� �Ӱ谪(%f)", result_invalid_ratio_threshold);
		system("pause");
		exit(1);
	}
}