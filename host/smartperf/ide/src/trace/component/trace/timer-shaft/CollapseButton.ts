import {BaseElement, element} from "../../../../base-ui/BaseElement.js";
import "../../../../base-ui/BaseElement.js";
import "../../../../base-ui/icon/LitIcon.js";

@element('collapse-button')
export default class CollapseButton extends BaseElement {
    static get observedAttributes() {
        return [
            'expand', //展开
        ];
    }

    set expand(value:boolean){
        if (value) {
            this.setAttribute('expand', '');
        }else{
            this.removeAttribute('expand');
        }
    }

    get expand(){
        return this.hasAttribute('expand');
    }

    initElements(): void {
        this.onclick=(e)=>{
            this.expand = !this.expand
            window.publish(window.SmartEvent.UI.CollapseAllLane, this.expand);
        }
    }

    initHtml(): string {
        return `
<style>
:host{
    position: absolute;
    left: 0;
    bottom: 0;
}
:host div{
    display: flex;
    padding: 0 6px;
    /*background-color: #00a3f5;*/
}
:host(:not([expand])) div{
    flex-direction: column;
}
:host([expand]) div{
    flex-direction: column-reverse;
}
div:hover{
    cursor: pointer;
}
</style>
<div>
    <lit-icon name="up" size="12"></lit-icon>
    <lit-icon name="down" size="12"></lit-icon>
</div>`;
    }

}