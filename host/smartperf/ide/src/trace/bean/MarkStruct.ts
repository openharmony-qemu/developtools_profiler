export class MarkStruct {
  startTimeStr: string | undefined;
  endTimeStr: string | undefined;
  startTime: number | undefined;
  endTime: number | undefined;
  colorEl: HTMLInputElement | undefined;
  operate: HTMLButtonElement | undefined;
  isSelected: boolean = false;
  constructor(
    operate: HTMLButtonElement,
    colorEl?: HTMLInputElement | undefined,
    startTimeStr?: string,
    startTime?: number,
    endTimeStr?: string,
    endTime?: number
  ) {
    this.operate = operate;
    this.colorEl = colorEl;
    this.startTimeStr = startTimeStr;
    this.startTime = startTime;
    this.endTimeStr = endTimeStr;
    this.endTime = endTime;
  }
}
