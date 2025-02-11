/*                                                                           */
/*   DemoPlay.cpp   デモプレイ処理                                           */
/*                                                                           */
/*                                                                           */

#include "platform/file.h"
#include "LZ_UTY.H"
#include "DEMOPLAY.H"


BOOL			DemoplaySaveEnable = FALSE;		// デモプレイのセーブが動作しているか
BOOL			DemoplayLoadEnable = FALSE;		// デモプレイのロードが動作しているか
DEMOPLAY_INFO	DemoInfo;						// デモプレイ情報
WORD			DemoBuffer[DEMOBUF_MAX];		// デモプレイ用バッファ
DWORD			DemoBufPtr;						// デモプレイ用バッファPtr(Ｃのそれではない)
CONFIG_DATA		ConfigTemp;						// コンフィグのデータ一時保存用

//DWORD RndBuf[RNDBUF_MAX];



PATH_STRING ReplayFN(uint8_t stage)
{
	if(stage == GRAPH_ID_EXSTAGE) {
		return PATH_STRING{ _PATH("秋霜りぷEx.DAT") };
	}

	auto ret = PATH_STRING{ _PATH("秋霜りぷ_.DAT") };
	ret[ret.size() - 5] = ('0' + GameStage);
	return ret;
}

// デモプレイデータの準備 //
FVOID DemoplayInit(void)
{
	// 乱数の準備 //
	DemoInfo.RndSeed = (DWORD)(rnd()+1)*(DWORD)(rnd()+1);
	rnd_seed_set(DemoInfo.RndSeed);

	DemoInfo.Exp    = Viv.exp;
	DemoInfo.Weapon = Viv.weapon;
	DemoInfo.CfgDat = ConfigDat;

	DemoBufPtr         = 0;
	DemoplaySaveEnable = TRUE;
}


// デモプレイデータを保存する //
FVOID DemoplayRecord(WORD key)
{
	if(!DemoplaySaveEnable) return;

	DemoBuffer[DemoBufPtr++] = key;

	// バッファが最後に来たか、ＥＳＣが押された場合 //
	if(DemoBufPtr==DEMOBUF_MAX || (key&KEY_ESC)){
		DemoBufPtr--;
		DemoplaySave();
		DemoplaySaveEnable = FALSE;
	}
}


// デモプレイデータをファイルに書き込む //
FVOID DemoplaySave(void)
{
	if(!DemoplaySaveEnable) return;

	DemoBuffer[DemoBufPtr] = KEY_ESC;
	DemoInfo.DataSize = DemoBufPtr+1;

	PATH_CODEPOINT fn[] = _PATH("STG_Demo.DAT");
	fn[3] = ('0' + GameStage);

	const std::array<BYTE_BUFFER_BORROWED, 2> bufs = {
		DemoInfo,
		std::span<WORD>{ DemoBuffer, (DemoBufPtr + 1) },
	};
	FileWrite(fn, std::span<const BYTE_BUFFER_BORROWED>{ bufs });

	DemoplaySaveEnable = FALSE;
}


// デモプレイデータをロードする //
FBOOL DemoplayLoad(int stage)
{
	// Not too pretty… //
	BYTE_BUFFER_OWNED DATEnemyLoad(fil_no_t filno);

	// 展開 //
	const auto temp = DATEnemyLoad(stage-1+18);
	auto temp_cursor = temp.cursor();
	{
		const auto maybe_info = temp_cursor.next<DEMOPLAY_INFO>();
		if(!maybe_info) {
			return false;
		}
		DemoInfo = maybe_info.value()[0];
	}
	{
		const auto maybe_inputs = temp_cursor.next<uint16_t>(DemoInfo.DataSize);
		if(!maybe_inputs) {
			return false;
		}
		const auto& inputs = maybe_inputs.value();
		memcpy(DemoBuffer, inputs.data(), inputs.size_bytes());
	}

	DemoBufPtr = 0;
	DemoplayLoadEnable = TRUE;

	// コンフィグの初期化 //
	ConfigTemp = ConfigDat;

	ConfigDat.BombStock   = DemoInfo.CfgDat.BombStock;
	ConfigDat.GameLevel   = DemoInfo.CfgDat.GameLevel;
	ConfigDat.PlayerStock = DemoInfo.CfgDat.PlayerStock;
	ConfigDat.InputFlags  = DemoInfo.CfgDat.InputFlags;

	Viv.exp    = DemoInfo.Exp;
	Viv.weapon = DemoInfo.Weapon;
	Viv.left   = ConfigDat.PlayerStock;
	Viv.bomb   = ConfigDat.BombStock;

	// 乱数の初期化 //
	rnd_seed_set(DemoInfo.RndSeed);

	return TRUE;
}


// Key_Data を返す //
FWORD DemoplayMove(void)
{
	DWORD ptr;

	if(!DemoplayLoadEnable) return KEY_ESC;

	ptr = DemoBufPtr;
	if(ptr>=DemoInfo.DataSize){
		ptr = DemoBufPtr;
		DemoplayLoadEnable = FALSE;
	}
	else DemoBufPtr++;

	return DemoBuffer[ptr];
}


// デモプレイロードの事後処理 //
FVOID DemoplayLoadCleanup(void)
{
	ConfigDat          = ConfigTemp;
	DemoplayLoadEnable = FALSE;
}


// リプレイデータの準備 //
FVOID ReplayInit(void)
{
	// これは同じ //
	DemoplayInit();
}


// リプレイデータを保存する //
FVOID ReplayRecord(WORD key)
{
	if(!DemoplaySaveEnable) return;

	DemoBuffer[DemoBufPtr++] = key;

	if(DemoBufPtr == DEMOBUF_MAX || (key&KEY_ESC)){
		DemoBufPtr--;
		ReplaySave();					// 強制吐き出し
		DemoplaySaveEnable = FALSE;
	}
}


// リプレイデータをファイルに書き込む //
FVOID ReplaySave(void)
{
	char			buf[100];

	if(!DemoplaySaveEnable) return;

	// すぐさま、無効化する //
	DemoplaySaveEnable = FALSE;

	DemoBuffer[DemoBufPtr] = KEY_ESC;
	DemoInfo.DataSize = DemoBufPtr+1;

	const auto demo_size = (sizeof(WORD) * DemoInfo.DataSize);
	PACKFILE_WRITE out = { {
		DemoInfo,
		std::span<WORD>{ DemoBuffer, demo_size },
	} };

	const auto fn = ReplayFN(GameStage);
	out.Write(fn.c_str());
}


// リプレイデータをロードする //
FBOOL ReplayLoad(int stage)
{
	BYTE_BUFFER_OWNED	temp;

	const auto fn = ReplayFN(GameStage);
	const auto in = FilStartR(fn.c_str());

	// ヘッダの格納先は０番である //
	temp = in.MemExpand( 0);
	if(NULL == temp) return FALSE;
	memcpy(&DemoInfo, temp.get(), sizeof(DEMOPLAY_INFO));

	// データの格納先は１番ですね //
	temp = in.MemExpand( 1);
	if(NULL == temp){
		return FALSE;
	}
	memcpy(DemoBuffer, temp.get(), sizeof(WORD)*DemoInfo.DataSize);


	DemoBufPtr = 0;
	DemoplayLoadEnable = TRUE;

	// 現在のコンフィグを保持する //
	ConfigTemp = ConfigDat;

	// そのときのコンフィグを転送 //
	ConfigDat.BombStock   = DemoInfo.CfgDat.BombStock;
	ConfigDat.GameLevel   = DemoInfo.CfgDat.GameLevel;
	ConfigDat.PlayerStock = DemoInfo.CfgDat.PlayerStock;
	ConfigDat.InputFlags  = DemoInfo.CfgDat.InputFlags;

	// 本体の性能記述 //
	Viv.exp    = DemoInfo.Exp;
	Viv.weapon = DemoInfo.Weapon;
	Viv.left   = ConfigDat.PlayerStock;
	Viv.bomb   = ConfigDat.BombStock;

	// 最後に乱数もそろえる //
	rnd_seed_set(DemoInfo.RndSeed);

	return TRUE;
}


// Key_Data を返す //
FWORD ReplayMove(void)
{
	DWORD	ptr;

	if(!DemoplayLoadEnable) return KEY_ESC;

	ptr = DemoBufPtr;

	if(ptr >= DemoInfo.DataSize){
		ptr = DemoBufPtr;
		DemoplayLoadEnable = FALSE;
	}
	else
		DemoBufPtr++;

	return DemoBuffer[ptr];
}


// リプレイデータロードの事後処理 //
FVOID ReplayLoadCleanup(void)
{
	ConfigDat = ConfigTemp;
	DemoplayLoadEnable = FALSE;
}
