#include "GarbageCollector.h"

// Garbage Collecter ������ ���� Ŭ���� ����

GarbageCollector::GarbageCollector() 
{
	this->invalid_ratio_threshold = 1.0;
	this->RDY_v_flash_info_for_set_invalid_ratio_threshold = false; //������ �÷��� �޸� ���� ���� �Ϸ� �� true�� set
	this->gc_lazy_mode = true; //��� ���� ���� �� false�� set
}

GarbageCollector::~GarbageCollector() {}

void GarbageCollector::print_invalid_ratio_threshold()
{
	printf("Current Invalid Ratio Threshold : %f\n", this->invalid_ratio_threshold);
}

int GarbageCollector::scheduler(FlashMem** flashmem, int mapping_method) //main scheduling function for GC
{
	if ((*flashmem) == NULL || mapping_method == 0) //�÷��� �޸𸮰� �Ҵ�Ǿ��ְ�, ���� ����� ����� ��쿡�� ����
		return FAIL;

	if (this->RDY_v_flash_info_for_set_invalid_ratio_threshold == false) //Reorganization�� ���� ������ �÷��� �޸� ������ �غ���� �ʾ�����
	{
		//���� ��ȿȭ�� ��Ͽ� ���ؼ� Victim Block ť�� ���Ը� ����, �̿� ����, ť�� ���� �� ��� ��ü ó��
		this->enqueue_job(flashmem, mapping_method);
		
		if((*flashmem)->victim_block_queue->is_full() == true)
			this->all_dequeue_job(flashmem, mapping_method); //VIctim Block ť ���� ��� Victim Block�鿡 ���� ó��

		return SUCCESS;
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
	unsigned int written_sector_count = (*flashmem)->v_flash_info.written_sector_count; //���� �÷��� �޸��� ��ϵ� ���� ��
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

	bool flag_vq_is_full = (*flashmem)->victim_block_queue->is_full();
	bool flag_vq_is_empty = (*flashmem)->victim_block_queue->is_empty();

	//���� ���������� �����ִ� ��� ���� ������ ���� ���
	if (physical_free_space < (BLOCK_PER_SECTOR * SECTOR_INC_SPARE_BYTE))
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
			this->all_dequeue_job(flashmem, mapping_method); //VIctim Block ť ���� ��� Victim Block�鿡 ���� ó��
		else if (flag_vq_is_empty == true && mapping_method == 3) //Victim Block ť�� �� ���
			full_merge(flashmem, mapping_method); //���̺� ���� ��ü ��Ͽ� ���� ���� �� ��� Merge ���� (Log Algorithm�� ������ ���̺긮�� ������ ��쿡�� ����)
		else
		{
			/***
				��� ������ ��� Merge �Ұ���, Erase�� ����
				Overwrite �߻� �� �ش� ����� �׻� ��ȿȭ�ǹǷ�, ����(������)������ ��ȿȭ�� �����Ͱ� �������� �ʴ�.
				����, �� ��� ���� ���� ��������� ��� ����Ͽ��� �� �̻� ����� �Ұ����� ����̴�.
			***/
			printf("End with no Operation\n(All physical spaces are used)\n");
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
			printf("End with no Operation\n");
			return COMPLETE;
		}
		else if (flag_vq_is_full == false && flag_vq_is_empty == false) //Victim Block ť�� ���� �� ���� �ʰ�, ������� ���� ���
		{
			switch (this->gc_lazy_mode)
			{
			case true:
				//���ӵ� ���� �۾��� ���� Write Performance ����� ���Ͽ� Victim Block ť�� ���� �� ������ �ƹ��� �۾��� �������� �ʴ´�.
				printf("End with no Operation (Lazy Mode)\n");
				return COMPLETE;

			case false:
				this->one_dequeue_job(flashmem, mapping_method); //�ϳ��� �� �ͼ� ó��
				break;
			}
		}
		else //Victim Block ť�� ���� �� ���
			this->all_dequeue_job(flashmem, mapping_method); //��� Victim Block�� ���ͼ� ó��
	}

	printf("Success\n");
	return SUCCESS;
}


int GarbageCollector::one_dequeue_job(class FlashMem** flashmem, int mapping_method) //Victim Block ť�κ��� �ϳ��� Victim Block�� ���ͼ� ó��
{
	//��� ���� : �ش� Victim Block�� �׻� ��ȿȭ�Ǿ� �����Ƿ� �ܼ� Erase ����
	//���̺긮�� ���� : ��ȿȭ�� ����� ��� �ܼ� Erase, �ƴ� ��� Merge ����

	victim_element victim_block; //ť���� ��Ҹ� ���ͼ� ����
	F_FLASH_INFO f_flash_info; //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	/***
		��� ������ Overwrite�߻� �� �ش� PBN�� ������ ��ȿȭ, Victim Block���� ���� => �ش� PBN�� ���� Erase����
	
		Log Algorithm�� ������ ���̺긮�� ������ LBN�� ������ PBN1 �Ǵ� PBN2�� ��� �����Ͱ� ��ȿȭ�ɽÿ� Victim Block���� ���� => �ش� PBN�� ���� Erase ����
		LBN�� PBN1, PBN2 ��� �����Ǿ� �ְ�(��, �����̶� ����� ��ȿȭ��������), ��ϰ��� Ȯ���� ���� LBN�� Victim Block���� ���� => �ش� LBN�� ���� Merge ����
		---
		=> �̿� ����, Victim Block ť�� PBN(���� ��� ��ȣ)�� ���ؼ��� �׻� ��ȿȭ�� ����̹Ƿ� Erase ����
		LBN�ϰ��, LBN�� ������ PBN1�� PBN2�� ���� Merge ����
	***/
	if ((*flashmem)->victim_block_queue->dequeue(victim_block) == SUCCESS)
	{
		switch (mapping_method)
		{
		case 2: //��� ����
			if (victim_block.victim_block_invalid_ratio != 1.0) //Overwrite �߻� �� �׻� �ش� ����� ���� ��ȿȭ�ǹǷ� ��ȿ���� 1.0�� �ƴϸ� ����
				goto WRONG_INVALID_RATIO_ERR;
			
			Flash_erase(flashmem, victim_block.victim_block_num);

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
				if (victim_block.victim_block_invalid_ratio != 1.0)
					goto WRONG_INVALID_RATIO_ERR;

				Flash_erase(flashmem, victim_block.victim_block_num);
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
	unsigned int written_sector_count = (*flashmem)->v_flash_info.written_sector_count; //���� �÷��� �޸��� ��ϵ� ���� ��
	F_FLASH_INFO f_flash_info = (*flashmem)->get_f_flash_info(); //�÷��� �޸� ���� �� �����Ǵ� ������ ����

	/***
		���������� �����ִ� ��� ���� ���� = ��ü byte���� �� - (��ϵ� ���� �� * ���� �� ����Ʈ ��)
		=> ����ڿ��� �������� �ʴ� �뷮�̹Ƿ�, Spare Block�� ���Խ�Ų��.
	***/
	unsigned int physical_free_space = f_flash_info.storage_byte - (written_sector_count * SECTOR_INC_SPARE_BYTE);

	/***
		- �ּ� �Ӱ谪 0.03125 (��� �ϳ��� ���� �� ������ �Ű澲�� �ʰ� �ϳ��� �������� ��ȿȭ�Ǿ��� ���� ��ȿ��)
		- �ִ� �Ӱ谪 1.0 (��� �ϳ��� ���� ��� �� ������ ��(32)��ŭ ��ȿȭ�Ǿ��� ���� ��ȿ��)
	***/

	//��ü �뷮�� ���� ���������� �����ִ� ��� ���� ���� ������ �Ӱ谪 ����
	try
	{
		float result_invalid_ratio_threshold = (float)physical_free_space / (float)f_flash_info.storage_byte;

		if (result_invalid_ratio_threshold == 0) //��ü �뷮�� ���� ���������� �����ִ� ��� ������ ���� ���� ��
			this->invalid_ratio_threshold = 0.03125; //�ּ� �Ӱ谪 ���� (1������ ��ȿȭ�� ��ȿ��)
		else if (result_invalid_ratio_threshold > 0 && result_invalid_ratio_threshold <= 1)
			this->invalid_ratio_threshold = result_invalid_ratio_threshold;
		else //�߸��� �Ӱ谪
			throw result_invalid_ratio_threshold;
	}
	catch (float result_invalid_ratio_threshold)
	{
		fprintf(stderr, "���� : �߸��� �Ӱ谪(%f)", &result_invalid_ratio_threshold);
		system("pause");
		exit(1);
	}
}