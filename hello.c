#include "data.h"
#include <efi.h>
#include <efilib.h>
#include <efitcp.h>

#define Print(message) ST->ConOut->OutputString(ST->ConOut,message)


EFI_TCP4* g_Tcp4Protocol = NULL;

VOID LogError(EFI_STATUS Status,CHAR16* szFunctionName);
VOID ConnectHandler(EFI_EVENT Event,VOID* Context);
VOID TransmitHandler(EFI_EVENT Event,VOID* Context);

EFI_STATUS ConfigureTcp();
EFI_STATUS Connect();
EFI_STATUS SendMessage();

EFI_STATUS efi_main(EFI_HANDLE ImageHandle,EFI_SYSTEM_TABLE *SystemTable)
{

    EFI_STATUS Status = EFI_SUCCESS;
	EFI_INPUT_KEY Key;

    EFI_TCP4_COMPLETION_TOKEN token;
    EFI_TCP4_CONNECTION_STATE Tcp4State;

	ST = SystemTable;

    Print(L"Firmware Vendor : ");
	Status = ST->ConOut->OutputString(ST->ConOut,ST->FirmwareVendor);
    Print(L"\r\n");

	if(EFI_ERROR(Status))
    {
        LogError(Status,L"OutputString");
        return Status;
    }

	Status = ST->BootServices->LocateProtocol(&Tcp4Protocol,NULL,&g_Tcp4Protocol);
	if(Status == EFI_NOT_FOUND)
    {
        LogError(Status,L"LocateProtocol");
        return Status;
    }

    Status = ConfigureTcp();
    Status = Connect();
    if(EFI_ERROR(Status))
    {
        LogError(Status,L"Connect:WaitForEvent");
        return Status;
    }

    Status = SendMessage();
    if(EFI_ERROR(Status))
    {
        LogError(Status,L"SendMessage:WaitForEvent");
        return Status;
    }

    Status = ST->ConIn->Reset(ST->ConIn,FALSE);
	if(EFI_ERROR(Status))
		return Status;
	
	while((Status = ST->ConIn->ReadKeyStroke(ST->ConIn,&Key)) == EFI_NOT_READY);
	return Status;
}

VOID ConnectHandler(EFI_EVENT Event,VOID* Context)
{
    Print(L"ConnectHandler : Connection established\r\n");
}


VOID TransmitHandler(EFI_EVENT Event,VOID* Context)
{
    Print(L"TransmitHandler : Message send\r\n");
}

EFI_STATUS Connect()
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_TCP4_CONNECTION_TOKEN ConnectionToken;
    UINTN Index;

    Status = ST->BootServices->CreateEvent(EVT_NOTIFY_WAIT,
                                           TPL_CALLBACK,
                                           ConnectHandler,
                                           &g_Tcp4Protocol,
                                           &ConnectionToken.CompletionToken.Event);
    ConnectionToken.CompletionToken.Status = 0;

    if(EFI_ERROR(Status))
    {
        LogError(Status,L"CreateEvent");
        return Status;
    }

    Status = g_Tcp4Protocol->Connect(g_Tcp4Protocol,&ConnectionToken);

    if(EFI_ERROR(Status))
    {
        LogError(Status,L"Connect");
        return Status;
    }

    Status = ST->BootServices->WaitForEvent(1,&ConnectionToken.CompletionToken.Event,&Index);
    if(EFI_ERROR(Status))
        LogError(Status,L"WaitForEvent");

    Status = ST->BootServices->CloseEvent(ConnectionToken.CompletionToken.Event);
    if(EFI_ERROR(Status))
        LogError(Status,L"CloseEvent");

    return Status;
}

EFI_STATUS ConfigureTcp()
{
    EFI_TCP4_OPTION ControlOption;
    EFI_TCP4_CONFIG_DATA Tcp4ConfigData;
    EFI_STATUS Status = EFI_SUCCESS;

    Tcp4ConfigData.TypeOfService = 0;
    Tcp4ConfigData.TimeToLive = 0;
    Tcp4ConfigData.AccessPoint.UseDefaultAddress = FALSE;

    Tcp4ConfigData.AccessPoint.StationAddress.Addr[0] = 192;
    Tcp4ConfigData.AccessPoint.StationAddress.Addr[1] = 168;
    Tcp4ConfigData.AccessPoint.StationAddress.Addr[2] = 0;
    Tcp4ConfigData.AccessPoint.StationAddress.Addr[3] = 10;

    Tcp4ConfigData.AccessPoint.SubnetMask.Addr[0] = 255;
    Tcp4ConfigData.AccessPoint.SubnetMask.Addr[1] = 255;
    Tcp4ConfigData.AccessPoint.SubnetMask.Addr[2] = 255;
    Tcp4ConfigData.AccessPoint.SubnetMask.Addr[3] = 0;

    Tcp4ConfigData.AccessPoint.StationPort = 0;
    Tcp4ConfigData.AccessPoint.RemoteAddress.Addr[0] = 192;
    Tcp4ConfigData.AccessPoint.RemoteAddress.Addr[1] = 168;
    Tcp4ConfigData.AccessPoint.RemoteAddress.Addr[2] = 0;
    Tcp4ConfigData.AccessPoint.RemoteAddress.Addr[3] = 1; /* 192.168.0.1 */
    Tcp4ConfigData.AccessPoint.RemotePort = 2333;
    Tcp4ConfigData.AccessPoint.ActiveFlag = TRUE;

    ControlOption.ReceiveBufferSize = 512;
    ControlOption.SendBufferSize = 512;
    ControlOption.MaxSynBackLog = 0; /* 0 --> Implementation */
    ControlOption.ConnectionTimeout = 0;
    ControlOption.DataRetries = 0;
    ControlOption.FinTimeout = 1;
    ControlOption.TimeWaitTimeout = 1;
    ControlOption.KeepAliveProbes = 0;
    ControlOption.KeepAliveTime = 0;
    ControlOption.KeepAliveInterval = 0;
    ControlOption.EnableNagle = FALSE;
    ControlOption.EnableTimeStamp = FALSE;
    ControlOption.EnableWindowScaling = FALSE;
    ControlOption.EnableSelectiveAck = FALSE;
    ControlOption.EnablePAthMtuDiscovery = FALSE;

    Tcp4ConfigData.ControlOption = &ControlOption;

    Status = g_Tcp4Protocol->Configure(g_Tcp4Protocol,&Tcp4ConfigData);

    if(EFI_ERROR(Status))
        LogError(Status,L"Configure");

    return Status;
}

EFI_STATUS SendMessage()
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_TCP4_TRANSMIT_DATA Packet;
    EFI_TCP4_IO_TOKEN IOToken;
    UINTN Index;

    IOToken.CompletionToken.Status = Tcp4StateEstablished;
    Status = ST->BootServices->CreateEvent(EVT_NOTIFY_WAIT,
                                           TPL_CALLBACK,
                                           TransmitHandler,
                                           &g_Tcp4Protocol,
                                           &IOToken.CompletionToken.Event);
    if(EFI_ERROR(Status))
    {
        LogError(Status,L"CreateEvent");
        return Status;
    }


    Packet.Push = TRUE;
    Packet.Urgent = FALSE;
    Packet.DataLength = 14;
    Packet.FragmentCount = 1;
    Packet.FragmentTable[0].FragmentLength = 14;
    Packet.FragmentTable[0].FragmentBuffer = "Hello, world\r\n";

    IOToken.Packet.TxData = &Packet;

    Status = g_Tcp4Protocol->Transmit(g_Tcp4Protocol,&IOToken);
    if(EFI_ERROR(Status))
    {
        LogError(Status,L"Transmit");
        return Status;
    }

    Status = ST->BootServices->WaitForEvent(1,&IOToken.CompletionToken.Event,&Index);
    if(EFI_ERROR(Status))
        LogError(Status,L"WaitForEvent");

    Status = ST->BootServices->CloseEvent(IOToken.CompletionToken.Event);
    if(EFI_ERROR(Status))
        LogError(Status,L"CloseEvent");

    return Status;
}


VOID LogError(EFI_STATUS Status,CHAR16* szFunctionName)
{
    Print(szFunctionName);
    switch (Status)
    {
        case EFI_NOT_FOUND:
            Print(L" : not found");
            break;
        case EFI_NOT_STARTED:
            Print(L" : not started\r\n");
            break;
        case EFI_NO_MAPPING:
            Print(L" : no mapping\r\n");
            break;
        case EFI_INVALID_PARAMETER:
            Print(L" : invalid parameter\r\n");
            break;
        case EFI_ACCESS_DENIED:
            Print(L" : access denied\r\n");
            break;
        case EFI_NOT_READY:
            Print(L" : not ready\r\n");
            break;
        case EFI_OUT_OF_RESOURCES:
            Print(L" : out of resources\r\n");
            break;
        case EFI_NETWORK_UNREACHABLE:
            Print(L" : network unreachable\r\n");
            break;
        case EFI_NO_MEDIA:
            Print(L" : no media\r\n");
            break;
        case EFI_UNSUPPORTED:
            Print(L" : unsupported\r\n");
            break;
        case EFI_DEVICE_ERROR:
            Print(L" : devicee error\r\n");
            break;
        default:
            Print(L" : unknown error\r\n");
    }
}
